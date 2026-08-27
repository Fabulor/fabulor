#!/usr/bin/env python3
"""Guard persistent sizing for the Ban List and Ignore List windows."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def source(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(needle: str, haystack: str) -> None:
    if needle not in haystack:
        raise AssertionError(f"Missing contract marker: {needle}")


def main() -> None:
    geometry = source("src/fe-gtk/window-geometry.c")
    banlist = source("src/fe-gtk/banlist.c")
    ban_view = source("src/fe-gtk/ban-list.c")
    ignore = source("src/fe-gtk/ignoregui.c")
    ignore_view = source("src/fe-gtk/ignore-list.c")

    unrealize = geometry.split("window_geometry_unrealize_cb", 1)[1].split(
        "window_geometry_watch_free", 1
    )[0]
    if "fabulor_window_geometry_get" in unrealize:
        raise AssertionError("Unrealize must not overwrite saved window geometry")
    win32_geometry = geometry.split("window_geometry_win32_proc", 1)[1].split(
        "#else", 1
    )[0]
    require("WM_EXITSIZEMOVE", win32_geometry)
    require("window_geometry_emit_current (watch)", win32_geometry)
    if "window_geometry_layout_cb" in win32_geometry:
        raise AssertionError("Win32 geometry must be captured after manual resizing")
    require(
        "gtk_window_set_default_size (GTK_WINDOW (banl->window), saved_width",
        banlist,
    )
    require(
        "gtk_window_set_default_size (GTK_WINDOW (ignorewin), saved_width",
        ignore,
    )
    for view in (ban_view, ignore_view):
        require("gtk_scrolled_window_set_propagate_natural_width", view)
        require("gtk_scrolled_window_set_propagate_natural_height", view)
        require("gtk_widget_set_size_request (scroller, 1, 1);", view)


if __name__ == "__main__":
    main()
