#!/usr/bin/env python
# -*- coding: utf-8 -*-

from websocket_server import WebsocketServer
from threading import Thread
from logging import *
from Config.Configures import config
import json

class WSS (Thread):

    def __init__(self, port=1883):
        Thread.__init__(self, name="WebSocketServer")
        self.isRunning = True

        self.server = WebsocketServer(port, loglevel=DEBUG)
        self.server.set_fn_new_client(self.new_client)
        self.server.set_fn_client_left(self.client_left)
        self.server.set_fn_message_received(self.message_received)

    # Called for every client connecting (after handshake)
    def new_client(self, client, server):
        print("New client connected and was given id %d" % client['id'])
        server.send_message_to_all("Hey all, a new client has joined us")


    # Called for every client disconnecting
    def client_left(self, client, server):
        print("Client(%d) disconnected" % client['id'])


    # Called when a client sends a message
    def message_received(self, client, server, message):
        if len(message) > 200:
            message = message[:200]+'..'
        print("Client(%d) said: %s" % (client['id'], message))
        server.send_message(client, message)

        print(json.loads(message)["zengjf"])

    def run(self):
        if self.isRunning:
            self.server.run_forever()

    def stop(self):
        self.isRunning = False
        self.server.server_close();

