#!/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.request


class MyHandler(BaseHTTPRequestHandler):
    data = ""

    def print_it(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        self.data = post_data.decode("utf8")
        print("== Received ==")
        print(self.raw_requestline)
        for header in self.headers:
            print("> {0}:{1}".format(header, self.headers[header]))
        print('-------------\n{0}\n-------------'.format(post_data))

    def do_GET(self):
        self.send_response(200, "OK")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

        self.print_it()
        return

    def publish(self, message):
        message = bytes(message, "utf8")
        url = "http://127.0.0.1:8000/"
        print(message)
        req = urllib.request.Request(url = url, data = message, method = 'POST')
        with urllib.request.urlopen(req):
            pass

    def do_POST(self):
        '''trigger Post output from arduino'''
        self.send_response(200, "OK")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

        self.print_it()
        message = "This is some random arduino output"
        self.publish(message)

        return

    def do_PUT(self):
        '''Received a command from server'''
        self.send_response(200, "OK")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

        self.print_it()

        if self.data == "command1":
            self.publish("This is an answer to command1")
        return


def run(server_class = HTTPServer, handler_class = MyHandler):
    server_address = ('', 8001)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()

run()
