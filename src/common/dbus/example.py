#!/usr/bin/python

# ZoiteChat
# Copyright (C) 1998-2010 Peter Zelezny.
# Copyright (C) 2009-2013 Berke Viktor.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#

from gi.repository import Gio

bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
connection = Gio.DBusProxy.new_sync(bus, Gio.DBusProxyFlags.NONE, None,
  						'org.zoitechat.service', '/org/zoitechat/Remote', 'org.zoitechat.connection', None)
path = connection.Connect('(ssss)', 
					'example.py',
					'Python example', 
					'Example of a D-Bus client written in python', 
					'1.0')		
zoitechat = Gio.DBusProxy.new_sync(bus, Gio.DBusProxyFlags.NONE, None,
								'org.zoitechat.service', path, 'org.zoitechat.plugin', None)
         
# Note the type before every arguement, this must be done.
# Type requirements are listed in our docs and characters are listed in the dbus docs.
# s = string, u = uint, i = int, etc.

channels = zoitechat.ListGet ('(s)', "channels")
while zoitechat.ListNext ('(u)', channels):
	name = zoitechat.ListStr ('(us)', channels, "channel")
	print("------- " + name + " -------")
	zoitechat.SetContext ('(u)', zoitechat.ListInt ('(us)', channels, "context"))
	zoitechat.EmitPrint ('(sas)', "Channel Message", ["John", "Hi there", "@"])
	users = zoitechat.ListGet ('(s)', "users")
	while zoitechat.ListNext ('(u)', users):
		print("Nick: " + zoitechat.ListStr ('(us)', users, "nick"))
	zoitechat.ListFree ('(u)', users)
zoitechat.ListFree ('(u)', channels)

print(zoitechat.Strip ('(sii)', "\00312Blue\003 \002Bold!\002", -1, 1|2))
