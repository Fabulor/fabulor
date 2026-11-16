"""Contracts for outgoing DCC readiness and ACK framing (no networking)."""
from pathlib import Path
import unittest
from test_dcc_sender_native import function
ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT/'src/common/dcc.c').read_text()

class DccSenderContracts(unittest.TestCase):
    def test_active_and_passive_use_unified_watch(self):
        for name in ['dcc_accept', 'dcc_connect_finished']:
            with self.subTest(name=name):
                body = function(SOURCE, name)
                self.assertIn('dcc_send_watch_update (dcc)', body)
                self.assertNotIn('dcc->wiotag = fe_input_add', body)
    def test_sender_has_one_readiness_source(self):
        body = function(SOURCE, 'dcc_send_watch_update')
        self.assertEqual(body.count('fe_input_add ('), 1)
        for guard in ['!dcc->throttled', 'dcc->pos < dcc->size', 'dcc->fastsend ||', 'dcc->send_watch_flags == flags']:
            self.assertIn(guard, body)
        self.assertIn('FIA_READ | FIA_EX', body)
    def test_dispatch_drains_ack_before_writing(self):
        body = function(SOURCE, 'dcc_send_ready')
        self.assertIn('G_IO_IN | G_IO_OUT | G_IO_HUP | G_IO_ERR', body)
        self.assertLess(body.index('dcc_read_ack ('), body.index('dcc_send_data ('))
        self.assertIn('dcc->dccstat == STAT_ACTIVE', body)
    def test_fragmented_ack_appends(self):
        self.assertIn('dcc->ack_buf + dcc->ack_pos, 4 - dcc->ack_pos', function(SOURCE, 'dcc_read_ack'))
    def test_sender_caps_reads_at_announced_end(self):
        body = function(SOURCE, 'dcc_send_data')
        self.assertIn('dcc->throttled || dcc->pos >= dcc->size', body)
        self.assertIn('dcc->size - dcc->pos', body)
        self.assertNotIn('wiotag', body)
        self.assertGreaterEqual(body.count('dcc_send_watch_update (dcc)'), 2)
    def test_close_removes_unified_watch(self):
        body = function(SOURCE, 'dcc_close')
        self.assertIn('dcc->send_watch_flags = 0', body)
        self.assertIn('fe_input_remove (dcc->iotag)', body)
        self.assertIn('dcc->iotag = 0', body)

if __name__ == '__main__': unittest.main()
