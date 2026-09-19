#!/usr/bin/env python3

import hashlib
import json
import math
import pathlib
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[2]
CONTRACT_PATH = ROOT / "tools" / "gtk4" / "pmv2-candidate-contract.json"


class Pmv2CandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.contract = json.loads(CONTRACT_PATH.read_text(encoding="utf-8"))
        cls.patch_path = ROOT / cls.contract["patch"]["path"]
        cls.patch = cls.patch_path.read_text(encoding="utf-8")
        cls.test_patch_path = ROOT / cls.contract["test_patch"]["path"]
        cls.test_patch = cls.test_patch_path.read_text(encoding="utf-8")
        cls.geometry_patch_path = ROOT / cls.contract["geometry_patch"]["path"]
        cls.geometry_patch = cls.geometry_patch_path.read_text(encoding="utf-8")

    def test_candidate_identity_and_patch_digest(self):
        self.assertEqual(self.contract["schema_version"], 1)
        self.assertEqual(self.contract["status"], "runtime-published")
        self.assertEqual(self.contract["activation"], "production")
        self.assertEqual(
            self.contract["candidate_dpi_awareness"], "per-monitor-v2"
        )
        self.assertEqual(
            self.contract["release_fallback"], "system-dpi-manifest-revert"
        )
        self.assertEqual(self.contract["gtk"]["version"], "4.22.4")
        self.assertEqual(self.contract["gtk"]["glib_version"], "2.88.0")
        self.assertEqual(
            self.contract["builder"]["release_tag"], "zoitechat-2.18.1"
        )
        self.assertEqual(
            self.contract["builder"]["cargo_c_version"],
            "0.10.24+cargo-0.98.0",
        )
        self.assertRegex(self.contract["gtk"]["source_commit"], r"^[0-9a-f]{40}$")
        self.assertRegex(self.contract["builder"]["base_commit"], r"^[0-9a-f]{40}$")
        self.assertEqual(
            self.contract["gtk"]["source_archive"]["sha256"],
            "51bd9f60c7d23a665a556c7364c21fb2e4e282566b3e7e092455e8f910330893",
        )

        digest = hashlib.sha256(self.patch_path.read_bytes()).hexdigest()
        self.assertEqual(digest, self.contract["patch"]["sha256"])
        test_digest = hashlib.sha256(self.test_patch_path.read_bytes()).hexdigest()
        self.assertEqual(test_digest, self.contract["test_patch"]["sha256"])
        geometry_digest = hashlib.sha256(
            self.geometry_patch_path.read_bytes()
        ).hexdigest()
        self.assertEqual(geometry_digest, self.contract["geometry_patch"]["sha256"])

    def test_fractional_scale_matrix_is_exact(self):
        scales = self.contract["acceptance_scales_percent"]
        self.assertEqual(scales, [100, 125, 150, 175, 200, 225, 250, 300])
        for percent in scales:
            dpi = 96 * percent // 100
            scale = max(dpi / 96.0, 1.0)
            with self.subTest(percent=percent):
                self.assertEqual(scale, percent / 100.0)
                self.assertEqual(math.ceil(scale), math.ceil(percent / 100.0))

    def test_candidate_runtime_archive_identity_is_isolated(self):
        archive = self.contract["runtime_archive"]
        dependency_contract = json.loads(
            (ROOT / archive["dependency_contract"]).read_text(encoding="utf-8")
        )
        source = dependency_contract["source"]

        self.assertEqual(archive["publication_status"], "published")
        self.assertEqual(source["url"], archive["url"])
        self.assertEqual(source["size_bytes"], archive["size_bytes"])
        self.assertEqual(source["sha256"], archive["sha256"])

        production_contract = json.loads(
            (ROOT / "tools" / "gtk4" / "dependency-contract.json").read_text(
                encoding="utf-8"
            )
        )
        self.assertEqual(production_contract["source"], source)

        components = json.loads(
            (ROOT / "third-party" / "components.json").read_text(encoding="utf-8")
        )["components"]
        gtk_component = next(item for item in components if item["id"] == "gtk4")
        self.assertEqual(gtk_component["distribution_sha256"], archive["sha256"])

        local_archive = ROOT / archive["local_path"]
        if local_archive.is_file():
            self.assertEqual(local_archive.stat().st_size, archive["size_bytes"])
            self.assertEqual(
                hashlib.sha256(local_archive.read_bytes()).hexdigest(),
                archive["sha256"],
            )

        native_test = self.contract["native_test_archive"]
        self.assertEqual(native_test["publication_status"], "published")
        self.assertEqual(native_test["release_tag"], archive["release_tag"])
        self.assertTrue(native_test["url"].endswith("/" + native_test["file_name"]))
        self.assertEqual(native_test["executable"], "win32-fractional-scale.exe")
        local_test = ROOT / native_test["local_path"]
        if local_test.is_file():
            self.assertEqual(local_test.stat().st_size, native_test["size_bytes"])
            self.assertEqual(
                hashlib.sha256(local_test.read_bytes()).hexdigest(),
                native_test["sha256"],
            )

    def test_coordinate_round_trips_and_repeated_transitions(self):
        scales = [
            percent / 100.0
            for percent in self.contract["acceptance_scales_percent"]
        ]
        logical_coordinates = [0, 1, 17, 145, 150, 256, 801, 1920]

        for logical in logical_coordinates:
            for scale in scales:
                physical = math.floor(logical * scale + 0.5)
                recovered = physical / scale
                with self.subTest(logical=logical, scale=scale):
                    self.assertLessEqual(
                        abs(recovered - logical), 0.5 / scale + 1e-9
                    )

        saved_width = 150.0
        current_width = saved_width
        for _ in range(25):
            for scale in scales + list(reversed(scales)):
                physical = math.floor(current_width * scale + 0.5)
                current_width = physical / scale
                self.assertLessEqual(
                    abs(current_width - saved_width), 4.0 / 3.0 + 1e-9
                )
        self.assertEqual(current_width, saved_width)

    def test_monitor_rectangles_use_outward_rounding(self):
        def logical_rect(left, top, right, bottom, scale):
            logical_left = math.floor(left / scale)
            logical_top = math.floor(top / scale)
            logical_right = math.ceil(right / scale)
            logical_bottom = math.ceil(bottom / scale)
            return (
                logical_left,
                logical_top,
                logical_right - logical_left,
                logical_bottom - logical_top,
            )

        self.assertEqual(logical_rect(-3840, 0, -1920, 2160, 2.5), (-1536, 0, 768, 864))
        self.assertEqual(logical_rect(-1919, -1079, -1, -1, 1.25), (-1536, -864, 1536, 864))
        self.assertEqual(logical_rect(1, 1, 1919, 1079, 1.75), (0, 0, 1097, 617))

    def test_patch_covers_win32_fractional_boundaries(self):
        required = (
            "double surface_scale;",
            "MAX ((double) dpix / USER_DEFAULT_SCREEN_DPI, 1.0)",
            'GetProcAddress (user32, "GetDpiForWindow")',
            'GetProcAddress (user32, "AdjustWindowRectExForDpi")',
            'g_object_notify (G_OBJECT (surface), "scale")',
            'g_object_notify (G_OBJECT (surface), "scale-factor")',
            "gdk_monitor_set_scale (monitor, impl->surface_scale)",
            "rect->right - rect->left",
            "SWP_NOACTIVATE | SWP_NOZORDER",
            "double                          scale;",
            "double           scale;",
        )
        for token in required:
            with self.subTest(token=token):
                self.assertIn(token, self.patch)
        added_lines = "\n".join(
            line[1:]
            for line in self.patch.splitlines()
            if line.startswith("+") and not line.startswith("+++")
        )
        self.assertNotIn(
            "return dpix / USER_DEFAULT_SCREEN_DPI > 1",
            added_lines,
        )

    def test_gtk_regression_patch_covers_conversions_and_transitions(self):
        required = (
            "gdk_win32_dpi_to_scale",
            "gdk_win32_physical_to_logical",
            "gdk_win32_logical_to_physical",
            "96, 120, 144, 168, 192, 216, 240, 288",
            'g_test_add_func ("/win32/fractional-scale/dpi"',
            'g_test_add_func ("/win32/fractional-scale/coordinates"',
            'g_test_add_func ("/win32/fractional-scale/transitions"',
        )
        for token in required:
            with self.subTest(token=token):
                self.assertIn(token, self.test_patch)

        geometry_required = (
            "gdk_win32_physical_rect_to_logical",
            "floor (left / scale)",
            "ceil (right / scale)",
            'g_test_add_func ("/win32/fractional-scale/monitor-rectangles"',
            "current_logical_width = gdk_win32_physical_to_logical",
        )
        for token in geometry_required:
            with self.subTest(token=token):
                self.assertIn(token, self.geometry_patch)

    def test_fabulor_handles_fractional_surface_notifications(self):
        source = (ROOT / "src" / "fe-gtk" / "maingui.c").read_text(
            encoding="utf-8"
        )
        self.assertIn('g_signal_connect (G_OBJECT (surface), "notify::scale"', source)
        self.assertIn("gtk_widget_queue_draw (gui->xtext);", source)
        self.assertIn("mg_schedule_rightpane_restore (gui);", source)

    def test_native_taskbar_geometry_remains_physical(self):
        source = (ROOT / "src" / "fe-gtk" / "window-state.c").read_text(
            encoding="utf-8"
        )
        self.assertIn("physical desktop pixels for a PMv2-aware HWND", source)
        self.assertIn("work_area = monitor_info.rcMonitor;", source)
        self.assertIn("SetWindowPos (hwnd, NULL, work_area.left", source)

    def test_candidate_manifest_activates_pmv2_with_fallbacks(self):
        manifest = ET.parse(ROOT / "win32" / "fabulor.exe.manifest").getroot()
        dpi_nodes = {
            node.tag.rsplit("}", 1)[-1]: (node.text or "").strip()
            for node in manifest.iter()
            if node.tag.rsplit("}", 1)[-1] in {"dpiAware", "dpiAwareness"}
        }
        self.assertEqual(dpi_nodes["dpiAware"], "true/pm")
        self.assertEqual(dpi_nodes["dpiAwareness"], "PerMonitorV2,PerMonitor")


if __name__ == "__main__":
    unittest.main()
