"""Source contracts complement the native Windows Sysinfo backend tests."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class SysinfoIntegration(unittest.TestCase):
    def test_missing_plugin_queries_are_wired_to_core(self):
        source = (ROOT / 'plugins/sysinfo/win32/backend.c').read_text()
        for name, target in [('sound', 'sound'), ('network', 'network'), ('chipset', 'chipset')]:
            self.assertIn(f'sysinfo_backend_get_{name} (void)\n{{\n\treturn sysinfo_get_{target} ();', source)

    def test_ram_alias_keeps_existing_memory_preference(self):
        source = (ROOT / 'plugins/sysinfo/sysinfo.c').read_text()
        self.assertIn('if (!g_ascii_strcasecmp (info, "ram"))\n\t\tinfo = "memory";', source)
        self.assertIn('{"memory", "Memory", sysinfo_backend_get_memory}', source)
        self.assertNotIn('{"ram",', source)
        self.assertIn('Sysinfo: %s information is unavailable', source)

    def test_device_queries_do_not_collect_network_identifiers(self):
        source = (ROOT / 'src/common/sysinfo/win32/backend.c').read_text()
        self.assertIn('SELECT Name FROM Win32_SoundDevice', source)
        self.assertIn('SELECT Name FROM Win32_NetworkAdapter WHERE PhysicalAdapter = TRUE', source)
        self.assertNotIn('MACAddress', source)
        self.assertNotIn('IPAddress', source)
        self.assertNotIn('Win32_NetworkAdapterConfiguration', source)
        self.assertIn('WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY', source)
        self.assertNotIn('WBEM_INFINITE', source)


if __name__ == '__main__':
    unittest.main()
