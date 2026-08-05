#!/usr/bin/env python3

import pathlib
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
ABSTRACTIONS = REPO_ROOT / "src" / "managed" / "Fabulor.PluginAbstractions"
HOST_SOURCE = (
    REPO_ROOT / "src" / "managed" / "Fabulor.PluginHost" / "NativeExports.cs"
)
SAMPLES = (
    REPO_ROOT / "samples" / "plugins" / "example.csharp.greeter" / "GreeterPlugin.cs",
    REPO_ROOT
    / "samples"
    / "plugins"
    / "simple-csharp-greeter"
    / "SimpleCSharpGreeter.cs",
)
CURRENT_GUIDES = (
    REPO_ROOT / "docs" / "plugins" / "csharp-plugin-guide.md",
    REPO_ROOT / "docs" / "plugins" / "plugin-schema-and-troubleshooting.md",
    REPO_ROOT / "docs" / "plugins" / "simple-addons.md",
)
PUBLIC_TYPES = (
    "IFabulorPlugin",
    "FabulorContext",
    "FabulorEvent",
    "FabulorEventHandler",
    "FabulorConsumingEventHandler",
    "FabulorEventResult",
    "FabulorUserInfo",
)
RETIRED_PREFIX = "Zoite" "Chat"
RETIRED_TYPES = (
    f"I{RETIRED_PREFIX}Plugin",
    f"{RETIRED_PREFIX}Context",
    f"{RETIRED_PREFIX}Event",
    f"{RETIRED_PREFIX}EventHandler",
    f"{RETIRED_PREFIX}UserInfo",
)


class CSharpApiContractTests(unittest.TestCase):
    def test_public_fabulor_types_are_declared(self):
        source = "\n".join(
            path.read_text(encoding="utf-8") for path in ABSTRACTIONS.glob("*.cs")
        )
        for type_name in PUBLIC_TYPES:
            with self.subTest(type_name=type_name):
                self.assertIn(type_name, source)

    def test_retired_managed_types_and_files_are_absent(self):
        surfaces = (
            *ABSTRACTIONS.glob("*.cs"),
            HOST_SOURCE,
            *SAMPLES,
            *CURRENT_GUIDES,
        )
        for path in surfaces:
            source = path.read_text(encoding="utf-8")
            for type_name in RETIRED_TYPES:
                with self.subTest(path=path, type_name=type_name):
                    self.assertNotIn(type_name, source)

        for type_name in RETIRED_TYPES:
            self.assertFalse((ABSTRACTIONS / f"{type_name}.cs").exists())

    def test_host_and_samples_use_fabulor_contract(self):
        host = HOST_SOURCE.read_text(encoding="utf-8")
        self.assertIn("typeof(IFabulorPlugin)", host)
        self.assertIn("new FabulorContext(", host)
        self.assertIn("FabulorEvent.FromJson(", host)
        self.assertIn("FabulorEventResult.Consume", host)
        for sample in SAMPLES:
            source = sample.read_text(encoding="utf-8")
            with self.subTest(sample=sample):
                self.assertIn(": IFabulorPlugin", source)
                self.assertIn("FabulorContext", source)

    def test_consuming_callback_contract_is_backward_compatible(self):
        context = (ABSTRACTIONS / "FabulorContext.cs").read_text(encoding="utf-8")
        result = (ABSTRACTIONS / "FabulorEventResult.cs").read_text(encoding="utf-8")
        self.assertIn("Action<string, FabulorEventHandler>", context)
        self.assertIn("Action<string, FabulorConsumingEventHandler>", context)
        self.assertIn("Continue = 0", result)
        self.assertIn("Consume = 1", result)


if __name__ == "__main__":
    unittest.main()
