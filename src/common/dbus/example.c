#include <gio/gio.h>
#include <glib.h>
#include <stdlib.h>

#define DBUS_SERVICE "org.zoitechat.service"
#define DBUS_CONNECTION_PATH "/org/zoitechat"
#define DBUS_CONNECTION_INTERFACE "org.zoitechat.connection"
#define DBUS_PLUGIN_INTERFACE "org.zoitechat.plugin"

static gboolean
call_sync (GDBusProxy *proxy, const char *method, GVariant *params, GVariant **out)
{
	GError *error = NULL;
	GVariant *result;

	result = g_dbus_proxy_call_sync (proxy,
		method,
		params,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		&error);
	if (!result)
	{
		g_printerr ("%s failed: %s\n", method, error->message);
		g_clear_error (&error);
		return FALSE;
	}

	if (out)
		*out = result;
	else
		g_variant_unref (result);

	return TRUE;
}

int
main (int argc, char **argv)
{
	GDBusConnection *connection;
	GDBusProxy *connection_proxy;
	GDBusProxy *plugin_proxy;
	GVariant *connect_result = NULL;
	gchar *remote_path = NULL;
	gchar *command = NULL;
	const char *path_tmp;
	int status = EXIT_FAILURE;
	GError *error = NULL;

	connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
	if (!connection)
	{
		g_printerr ("Bus connection failed: %s\n", error->message);
		g_clear_error (&error);
		return EXIT_FAILURE;
	}

	connection_proxy = g_dbus_proxy_new_sync (connection,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		DBUS_SERVICE,
		DBUS_CONNECTION_PATH,
		DBUS_CONNECTION_INTERFACE,
		NULL,
		&error);
	if (!connection_proxy)
	{
		g_printerr ("Connection proxy failed: %s\n", error->message);
		g_clear_error (&error);
		g_object_unref (connection);
		return EXIT_FAILURE;
	}

	if (!call_sync (connection_proxy,
		"Connect",
		g_variant_new ("(ssss)", "example", "example", "GDBus example", "1.0"),
		&connect_result))
		goto cleanup;

	g_variant_get (connect_result, "(&s)", &path_tmp);
	remote_path = g_strdup (path_tmp);
	g_variant_unref (connect_result);
	connect_result = NULL;

	plugin_proxy = g_dbus_proxy_new_sync (connection,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		DBUS_SERVICE,
		remote_path,
		DBUS_PLUGIN_INTERFACE,
		NULL,
		&error);
	if (!plugin_proxy)
	{
		g_printerr ("Plugin proxy failed: %s\n", error->message);
		g_clear_error (&error);
		goto cleanup;
	}

	if (argc > 1)
		command = g_strjoinv (" ", &argv[1]);
	else
		command = g_strdup ("gui focus");

	if (!call_sync (plugin_proxy, "Command", g_variant_new ("(s)", command), NULL))
	{
		g_object_unref (plugin_proxy);
		goto cleanup;
	}

	call_sync (connection_proxy, "Disconnect", g_variant_new ("()"), NULL);
	status = EXIT_SUCCESS;
	g_object_unref (plugin_proxy);

cleanup:
	g_free (command);
	g_free (remote_path);
	g_object_unref (connection_proxy);
	g_object_unref (connection);
	return status;
}
