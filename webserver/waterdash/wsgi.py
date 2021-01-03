#!/usr/bin/env python3.9

import sys
from data.config import app_path

sys.path.insert(0, app_path)

from display_server import server as application

