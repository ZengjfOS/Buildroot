#!/usr/bin/env python
# -*- coding: utf-8 -*-

from Shell.ShellCmd import ShellCmd

class GPIO:
    def __init__(self, server, client, json_data):
        server.send_message(client, "GPIO")
