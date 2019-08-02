#!/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.request


class MyHandler(BaseHTTPRequestHandler):
    arduino_buffer = ""

    def do_GET(self):
        self.send_response(200, "OK")
        self.send_header('Content-type', 'text/html')
        self.end_headers()

        message = MyHandler.arduino_buffer
        self.wfile.write(bytes(message, "utf8"))
        MyHandler.arduino_buffer = ""
        return

    def do_POST(self):
        '''Post output from arduino'''
        self.send_response(200, "OK")
        self.send_header('Content-type', 'text/html')
        self.end_headers()

        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        MyHandler.arduino_buffer += post_data.decode("utf8") + "\n"
        return

    def do_PUT(self):
        '''Send commands to ARDUINO'''
        self.send_response(200, "OK")
        self.end_headers()

        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        url = "http://127.0.0.1:8001/"

        req = urllib.request.Request(url = url, data = post_data, method = 'PUT')
        with urllib.request.urlopen(req):
            pass


def run(server_class = HTTPServer, handler_class = MyHandler):
    server_address = ('', 8000)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()


run()
