#!/usr/bin/env python3
"""Guard persistent sizing for the Ban List and Ignore List windows."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def source(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def main() -> None:
    geometry = source("src/fe-gtk/window-geometry.c")
    banlist = source("src/fe-gtk/banlist.c")
    ban_view = source("src/fe-gtk/ban-list.c")
    ignore = source("src/fe-gtk/ignoregui.c")
    ignore_view = source("src/fe-gtk/ignore-list.c")

    unrealize = geometry.split("window_geometry_unrealize_cb", 1)[1].split(
        "window_geometry_watch_free", 1
    )[0]
    assert "fabulor_window_geometry_get" not in unrealize
    win32_geometry = geometry.split("window_geometry_win32_proc", 1)[1].split(
        "#else", 1
    )[0]
    assert "WM_EXITSIZEMOVE" in win32_geometry
    assert "window_geometry_emit_current (watch)" in win32_geometry
    assert "window_geometry_layout_cb" not in win32_geometry
    assert "gtk_window_set_default_size (GTK_WINDOW (banl->window), saved_width" in banlist
    assert "gtk_window_set_default_size (GTK_WINDOW (ignorewin), saved_width" in ignore
    for view in (ban_view, ignore_view):
        assert "gtk_scrolled_window_set_propagate_natural_width" in view
        assert "gtk_scrolled_window_set_propagate_natural_height" in view
        assert "gtk_widget_set_size_request (scroller, 1, 1);" in view


if __name__ == "__main__":
    main()
