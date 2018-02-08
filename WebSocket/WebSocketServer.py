#!/usr/bin/env python
# -*- coding: utf-8 -*-

from websocket_server import WebsocketServer
from threading import Thread
from logging import *
from Config.Configures import config
from Hardware.GPIO import GPIO
import json

class WSS (Thread):

    def __init__(self, address="0.0.0.0", port=1883):
        Thread.__init__(self, name="WebSocketServer")
        self.isRunning = True

        self.server = WebsocketServer(port, host=address, loglevel=DEBUG)
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
        info("Client(%d) said: %s" % (client['id'], message))

        self.message_parser(client, message)

    def run(self):
        if self.isRunning:
            self.server.run_forever()

    def stop(self):
        self.isRunning = False
        self.server.server_close();

    # {"categories":"hardware_gpio", "type": "output", "index": index, "value": val};
    def message_parser(self, client, message):
        try:
            parsed_message = json.loads(message)
        except:
            info("bad json data format.")
            return

        if parsed_message["categories"] in config.config["categories"]:
            eval(config.config["categories"][parsed_message["categories"]])(self.server, client, parsed_message)


