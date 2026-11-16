#include <enchant.h>
#include <enchant-provider.h>
#include <gmodule.h>
#include <stdio.h>
#include <string.h>

/* Test-only access to the pinned 2.8.19 provider ABI, for fail-closed preflight. */
extern void enchant_provider_dict_unref(gpointer instance);
extern void enchant_provider_unref(gpointer instance);

static EnchantDict *request_english(EnchantBroker *broker)
{
    static const char *tags[] = {"en_AU", "en-US", "en_US", "en", NULL};
    int i;
    for (i = 0; tags[i]; i++) {
        EnchantDict *dict = enchant_broker_request_dict(broker, tags[i]);
        if (dict) return dict;
    }
    return NULL;
}

static int safe_provider(const char *path)
{
    GModule *module = g_module_open(path, G_MODULE_BIND_LOCAL);
    gpointer symbol = NULL;
    EnchantProvider *provider;
    EnchantProviderDict *dict;
    EnchantProvider *(*init)(void);
    int safe;
    if (!module || !g_module_symbol(module, "init_enchant_provider", &symbol)) return 0;
    init = (EnchantProvider *(*)(void)) symbol;
    provider = init();
    if (!provider) { g_module_close(module); return 0; }
    dict = provider->request_dict(provider, "en_AU");
    if (!dict) dict = provider->request_dict(provider, "en_US");
    /* Never exercise session APIs if a provider can call persistent Windows
     * Add/Remove. This guard must precede all word-changing tests. */
    safe = dict && !dict->add_to_session && !dict->remove_from_session;
    if (dict) enchant_provider_dict_unref(dict);
    enchant_provider_unref(provider);
    g_module_close(module);
    return safe;
}

static int isolated_config(void)
{
    const char *configured = g_getenv("ENCHANT_CONFIG_DIR");
    char *actual, *a, *b;
    int equal;
    if (!configured || !*configured) return 0;
    actual = enchant_get_user_config_dir();
    if (!actual) return 0;
    a = g_canonicalize_filename(configured, NULL);
    b = g_canonicalize_filename(actual, NULL);
    equal = g_ascii_strcasecmp(a, b) == 0;
    g_free(a); g_free(b); g_free(actual);
    return equal;
}

int main(int argc, char **argv)
{
    const char *personal_word = "fabulorisolatedpersonalqzxv";
    const char *session_word = "fabulorsessiononlyqzxv";
    EnchantBroker *broker, *other;
    EnchantDict *dict, *other_dict;
    char **suggestions;
    size_t count = 0;
    if (argc != 2 || !isolated_config() || !safe_provider(argv[1])) {
        fputs("Unsafe provider or missing isolated config: no word tests performed\n", stderr);
        return 10;
    }
    broker = enchant_broker_init();
    if (!broker || strcmp(enchant_get_version(), "2.8.19") != 0) return 1;
    dict = request_english(broker);
    if (!dict) return 2;
    if (enchant_dict_check(dict, session_word, -1) != 1 ||
        enchant_dict_check(dict, personal_word, -1) != 1 ||
        enchant_dict_check(dict, "the", -1) != 0) return 5;
    enchant_dict_add_to_session(dict, session_word, -1);
    enchant_dict_remove_from_session(dict, "the", -1);
    if (enchant_dict_check(dict, session_word, -1) != 0 ||
        enchant_dict_check(dict, "the", -1) != 1) return 6;
    other = enchant_broker_init();
    other_dict = request_english(other);
    if (!other_dict || enchant_dict_check(other_dict, session_word, -1) != 1 ||
        enchant_dict_check(other_dict, "the", -1) != 0) return 7;
    enchant_broker_free_dict(other, other_dict);
    enchant_broker_free(other);
    suggestions = enchant_dict_suggest(dict, "piples", -1, &count);
    enchant_dict_free_string_list(dict, suggestions);
    /* This deliberately tests persistence only inside ENCHANT_CONFIG_DIR. */
    enchant_dict_add(dict, personal_word, -1);
    if (enchant_dict_check(dict, personal_word, -1) != 0) return 3;
    enchant_broker_free_dict(broker, dict);
    enchant_broker_free(broker);
    broker = enchant_broker_init();
    dict = request_english(broker);
    if (!dict || enchant_dict_check(dict, personal_word, -1) != 0 ||
        enchant_dict_check(dict, session_word, -1) != 1 ||
        enchant_dict_check(dict, "the", -1) != 0) return 4;
    enchant_broker_free_dict(broker, dict);
    enchant_broker_free(broker);
    puts("Enchant MSVC smoke passed: session state stays local; isolated personal words persist");
    return 0;
}
