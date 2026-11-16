"""Optional native UI example: load explicitly, then /UIPANELDEMO.

No network, file writes, background workers, timers or GUI toolkit dependency.
The native Python host owns panel cleanup when this add-on unloads.
"""
import fabulor

__module_name__ = 'Native Panel Demo'
__module_version__ = '1.1'
__module_description__ = 'Optional native control-panel example; /UIPANELDEMO'


def panel_event(panel, event, values, userdata):
    # This entry-point module retains the loader's __plugin callback ownership.
    if event == 'apply':
        panel.update({'result': {'value': 'Applied locally: {}'.format(values['example']), 'state': 'running'}})


def open_panel(words, word_eol, userdata):
    if not getattr(fabulor, 'ui_available', lambda: False)():
        fabulor.prnt('Native panel bridge unavailable; requires updated frontend and Python host.')
        return fabulor.EAT_ALL
    schema = {
        'title': 'Native Panel Demo', 'context': 'Local demonstration - no IRC actions',
        'pages': [{'id': 'overview', 'title': 'Overview', 'fields': [
            {'id': 'result', 'type': 'status', 'label': 'Demo state', 'value': 'Waiting for Apply', 'state': 'waiting'},
            {'id': 'example', 'type': 'check', 'label': 'Example draft setting', 'value': False},
        ]}],
        'buttons': [{'id': 'apply', 'label': 'Apply locally'}],
    }
    if 'page_buttons' in getattr(fabulor, 'ui_features', lambda: ())():
        schema['pages'][0]['buttons'] = schema.pop('buttons')
    fabulor.ui_open(schema, panel_event)
    return fabulor.EAT_ALL


fabulor.hook_command('UIPANELDEMO', open_panel, help='UIPANELDEMO opens an optional native UI example.')
