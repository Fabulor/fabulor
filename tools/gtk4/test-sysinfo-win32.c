/* Native tests of the actual Windows backend, with synthetic WMI disk rows.
 * --live queries the local machine, printing availability only (no identifiers).
 */
#include "../../src/common/sysinfo/win32/backend.c"

static const wchar_t *fixture_free = L"0";
static const wchar_t *fixture_size = L"1000";
static VARTYPE fixture_type = VT_BSTR;

static HRESULT STDMETHODCALLTYPE
fixture_get (IWbemClassObject *self, LPCWSTR name, LONG flags,
             VARIANT *value, CIMTYPE *type, LONG *flavour)
{
	(void) self; (void) flags; (void) type; (void) flavour;
	VariantInit (value);
	if (!wcscmp (name, L"Name"))
	{
		V_VT (value) = VT_BSTR;
		value->bstrVal = SysAllocString (L"C:");
	}
	else
	{
		V_VT (value) = fixture_type;
		if (fixture_type == VT_BSTR)
			value->bstrVal = SysAllocString (!wcscmp (name, L"Size") ? fixture_size : fixture_free);
	}
	return S_OK;
}

static void test_disk_rows (void)
{
	IWbemClassObjectVtbl vtable = { 0 };
	IWbemClassObject object = { &vtable };
	vtable.Get = fixture_get;
	hdd_capacity = hdd_free_space = 0;
	fixture_type = VT_BSTR;
	fixture_free = L"0";
	read_hdd_info (&object);
	g_assert_cmpuint (hdd_capacity, ==, 1000);
	g_assert_cmpuint (hdd_free_space, ==, 0);
	fixture_free = L"200";
	read_hdd_info (&object);
	g_assert_cmpuint (hdd_capacity, ==, 2000);
	g_assert_cmpuint (hdd_free_space, ==, 200);
	fixture_free = L"1001";
	read_hdd_info (&object);
	fixture_free = L"garbage";
	read_hdd_info (&object);
	fixture_type = VT_NULL;
	read_hdd_info (&object);
	g_assert_cmpuint (hdd_capacity, ==, 2000);
	g_assert_cmpuint (hdd_free_space, ==, 200);
	fixture_type = VT_BSTR;
	fixture_free = L"0";
	hdd_capacity = UINT64_MAX;
	read_hdd_info (&object);
	g_assert_cmpuint (hdd_capacity, ==, UINT64_MAX);
}

static void test_variants (void)
{
	VARIANT v;
	uint64_t value = 99;
	const wchar_t *invalid[] = { L"", L"-1", L"+1", L"1x", L"18446744073709551616" };
	unsigned int i;
	VariantInit (&v);
	V_VT (&v) = VT_BSTR;
	v.bstrVal = SysAllocString (L"0");
	g_assert_true (variant_to_uint64 (&v, &value));
	g_assert_cmpuint (value, ==, 0);
	VariantClear (&v);
	for (i = 0; i < G_N_ELEMENTS (invalid); i++)
	{
		V_VT (&v) = VT_BSTR;
		v.bstrVal = SysAllocString (invalid[i]);
		g_assert_false (variant_to_uint64 (&v, &value));
		VariantClear (&v);
	}
	V_VT (&v) = VT_UI8;
	v.ullVal = UINT64_MAX;
	g_assert_true (variant_to_uint64 (&v, &value));
	g_assert_cmpuint (value, ==, UINT64_MAX);
	V_VT (&v) = VT_NULL;
	g_assert_false (variant_to_uint64 (&v, &value));
}

static void test_names (void)
{
	BSTR name = SysAllocString (L"Audio\r\n\x0003 Device \u00e9");
	char *result = bstr_to_utf8 (name);
	g_assert_cmpstr (result, ==, "Audio Device \303\251");
	free (result);
	SysFreeString (name);
	g_assert_null (bstr_to_utf8 (NULL));
}

int main (int argc, char **argv)
{
	if (argc == 2 && !strcmp (argv[1], "--live"))
	{
		char *(*getters[]) (void) = { sysinfo_get_sound, sysinfo_get_network,
			sysinfo_get_chipset, sysinfo_get_cpu, sysinfo_get_gpu };
		const char *labels[] = { "sound", "ethernet", "chipset", "cpu", "gpu" };
		unsigned int i;
		HRESULT hr = CoInitializeEx (NULL, COINIT_APARTMENTTHREADED);
		if (FAILED (hr)) return 1;
		hr = CoInitializeSecurity (NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
			RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
		printf ("COM security: 0x%08lx\n", (unsigned long) hr);
		for (i = 0; i < G_N_ELEMENTS (getters); i++)
		{
			char *value = getters[i] ();
			printf ("%s: %s\n", labels[i], value && *value ? "available" : "unavailable");
			free (value);
		}
		CoUninitialize ();
		return 0;
	}
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/sysinfo/disk/full-and-invalid-rows", test_disk_rows);
	g_test_add_func ("/sysinfo/variants/unsigned-values", test_variants);
	g_test_add_func ("/sysinfo/names/irc-controls-and-utf8", test_names);
	return g_test_run ();
}
