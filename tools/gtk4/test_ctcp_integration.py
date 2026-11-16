"""Offline source contracts for CTCP echo and system-string handling."""
from pathlib import Path
import unittest
from test_dcc_sender_native import function
ROOT=Path(__file__).resolve().parents[2]
CTCP=(ROOT/'src/common/ctcp.c').read_text()
PROTO=(ROOT/'src/common/proto-irc.c').read_text()
UTIL=(ROOT/'src/common/util.c').read_text()

class CtcpContracts(unittest.TestCase):
    def test_classifier_requires_capability_and_own_sender(self):
        body=function(CTCP,'ctcp_is_own_echo')
        self.assertIn('serv->have_echo_message',body)
        self.assertIn('!serv->p_cmp (nick, serv->nick)',body)
        self.assertIn('serv->p_cmp (to, serv->nick)',body)
    def test_action_echo_is_preserved(self):
        self.assertIn('g_ascii_strncasecmp (msg, "ACTION ", 7) != 0',function(CTCP,'ctcp_is_own_echo'))
    def test_guard_precedes_dcc_and_reply_dispatch(self):
        body=function(CTCP,'ctcp_handle')
        guard=body.index('ctcp_is_own_echo (')
        self.assertLess(guard,body.index('handle_dcc ('))
        self.assertLess(guard,body.index('ctcp_check ('))
        self.assertLess(guard,body.index('serv->p_nctcp ('))
    def test_protocol_guard_precedes_flood_and_dcc_reparse(self):
        body=PROTO[PROTO.index("case WORDL('P','R','I','V')"):]
        guard=body.index('ctcp_is_own_echo (')
        self.assertLess(guard,body.index('flood_check ('))
        self.assertLess(guard,body.index('process_data_init ('))
    def test_notice_is_not_a_request(self):
        body=PROTO[PROTO.index("case WORDL('N','O','T','I')"):PROTO.index("case WORDL('P','R','I','V')")]
        self.assertIn('inbound_notice (',body)
        self.assertNotIn('ctcp_handle (',body)
    def test_missing_cpu_has_no_brackets(self):
        body=function(UTIL,'get_sys_str')
        self.assertIn('(cpu && cpu[0])',body)
        self.assertIn(': g_strdup (os)',body)
        self.assertIn('g_free (cpu)',body)
        self.assertIn('g_free (os)',body)

if __name__=='__main__':unittest.main()
