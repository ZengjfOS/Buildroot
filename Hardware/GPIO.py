#!/usr/bin/env python
# -*- coding: utf-8 -*-

from Shell.ShellCmd import ShellCmd
import json

class GPIO:
    def __init__(self, server, client, json_data):
        ret = json_data
        if json_data["type"] == "output":
            ret["result"] = ShellCmd.execute("gpiotool -p mio -m o -i " + str(json_data["index"]) + " -v " + str(json_data["value"]) + " | grep -E '^(0|1|-1)'")[0].strip()
            ret["status"] = "ok"
            pass
        elif json_data["type"] == "input":
            ret["gpios"] = {}
            for index in range(4):
                ret["gpios"]["in_" + str(index)] = {}
                shell_ret = ShellCmd.execute("gpiotool -p mio -m i -i " + str(index + 4)+ " | grep -E '^(0|1|-1)'")
                if shell_ret[0] == "0\n" or shell_ret[0] == "1\n":
                    ret["gpios"]["in_" + str(index)]["status"] = "ok"
                else:
                    ret["gpios"]["in_" + str(index)]["status"] = "error"

                ret["gpios"]["in_" + str(index)]["value"] = shell_ret[0].strip()

            ret["status"] = "ok"
            pass
        else :
            ret["status"] = "error"
            pass

        server.send_message(client, json.dumps(ret))
