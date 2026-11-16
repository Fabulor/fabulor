"""Source contracts for production menu wiring; not live DALnet acceptance."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class DalnetMenuIntegration(unittest.TestCase):
    def test_permission_dispatch_uses_origin_and_validated_raw_line(self):
        source = (ROOT / "src/fe-gtk/menu.c").read_text(encoding="utf-8")
        block = source.split("if (action == FABULOR_NICK_CONTEXT_DCC_PERMISSION)", 1)[1]
        block = block.split("if (action == FABULOR_NICK_CONTEXT_COPY_INFO)", 1)[0]
        for text in ("sess = popup->session", "!is_session (sess)",
                     "sess->server->id != popup->server_id", "!sess->server->connected",
                     "menu_nick_server_is_dalnet", "popup->num_sel > 1",
                     "fabulor_nick_context_dccallow_command", "sess->server->p_raw"):
            self.assertIn(text, block)
        self.assertNotIn("nick_command_parse (", block)
        self.assertNotIn("userlist_button_cb (", block)
        self.assertNotIn("sess = current_sess", block)

    def test_reported_network_survives_custom_bouncer_label(self):
        source = (ROOT / "src/fe-gtk/menu.c").read_text(encoding="utf-8")
        policy = source.split("menu_nick_server_is_dalnet (server *serv)", 1)[1]
        policy = policy.split("static GWeakRef", 1)[0]
        self.assertIn("server_get_network (serv, FALSE)", policy)
        self.assertIn("serv->servername", policy)
        self.assertIn("serv->server_session->channel", policy)
        self.assertIn("serv->server_session->type == SESS_SERVER", policy)

    def test_existing_popup_configuration_is_not_replaced(self):
        source = (ROOT / "src/fe-gtk/menu.c").read_text(encoding="utf-8")
        self.assertIn("for (list = popup_list; list; list = list->next)", source)
        self.assertIn("handlers = menu_nick_handlers_snapshot (sess, command_target)", source)
        self.assertIn("g_array_append_vals (handlers, permissions", source)
        self.assertIn("sess->server->connected && popup->num_sel <= 1", source)
        self.assertIn("popup->session = sess", source)
        self.assertIn("popup->server_id = sess->server->id", source)


if __name__ == "__main__":
    unittest.main()
