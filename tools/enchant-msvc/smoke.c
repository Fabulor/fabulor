#include <enchant.h>
#include <stdio.h>
#include <string.h>

static EnchantDict *request_english(EnchantBroker *broker)
{
    static const char *tags[] = {"en_AU", "en-US", "en_US", "en", NULL};
    int i;
    for (i = 0; tags[i]; i++)
    {
        EnchantDict *dict = enchant_broker_request_dict(broker, tags[i]);
        if (dict)
            return dict;
    }
    return NULL;
}

int main(void)
{
    const char *personal_word = "fabulorcrttestword";
    EnchantBroker *broker = enchant_broker_init();
    EnchantDict *dict;
    char **suggestions;
    size_t count = 0;

    if (!broker || strcmp(enchant_get_version(), "2.8.19") != 0)
        return 1;
    dict = request_english(broker);
    if (!dict)
        return 2;
    (void)enchant_dict_check(dict, "https", -1);
    (void)enchant_dict_check(dict, "piples", -1);
    suggestions = enchant_dict_suggest(dict, "piples", -1, &count);
    enchant_dict_free_string_list(dict, suggestions);
    enchant_dict_add(dict, personal_word, -1);
    if (enchant_dict_check(dict, personal_word, -1) != 0)
        return 3;
    enchant_broker_free_dict(broker, dict);
    enchant_broker_free(broker);

    broker = enchant_broker_init();
    dict = request_english(broker);
    if (!dict || enchant_dict_check(dict, personal_word, -1) != 0)
        return 4;
    enchant_broker_free_dict(broker, dict);
    enchant_broker_free(broker);
    puts("Enchant MSVC smoke test passed");
    return 0;
}
