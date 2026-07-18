# Notification Architecture

Status: Stage 7 pass 6 complete for backend loading and lifecycle containment

## Boundary

The built-in Notifications plugin owns one frontend-selected backend. Backend
initialization returns an owned `GError`; plugin initialization logs and frees
that error before declining to register notification hooks. Backend teardown is
safe after successful initialization and repeated initialization does not
create a second platform owner.

On Windows, `notification-windows.c` is the only owner of
`hcnotifications-winrt.dll`. The helper is resolved from the running
executable's installation directory and `plugins` subdirectory. It is not
searched through the process working directory, `PATH`, or the user add-on
loader.

The Windows owner retains the `GModule` handle and validates all four required
exports before invoking any helper code:

- `notification_backend_init`
- `notification_backend_deinit`
- `notification_backend_show`
- `notification_backend_supported`

Missing exports and helper initialization failures clear every function
pointer and unload the module. Normal teardown releases the WinRT notifier,
balances Windows Foundation initialization, clears callable pointers, and then
unloads the module.

## Platform Implementations

The WinRT helper initializes Windows Foundation before creating its toast
notifier. Initialization and deinitialization are idempotent, and partial
notifier construction is unwound before failure is returned to the frontend.

The freedesktop backend retains its D-Bus proxy as the platform owner and
propagates initialization failures through `GError`. Teardown clears the proxy
and capability state. The dummy backend reports an owned unsupported-platform
error instead of failing without diagnostics.

## Deferred GTK4 Work

This pass hardens the existing platform boundary and does not introduce a GTK4
notification presentation surface. Production GTK4 validation still needs to
cover preference interaction, toast delivery, tray interaction, startup from
an unrelated working directory, repeated frontend startup/shutdown, and the
packaged helper DLL. Unix tray-backend selection remains a separate decision.
