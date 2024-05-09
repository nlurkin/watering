FROM python:3.11-slim

WORKDIR /app

COPY requirements.common.txt ./
RUN pip install --no-cache-dir -r requirements.common.txt

COPY mosquitto/requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY mqtt_server.py mongodb.py ./

CMD [ "python", "./mqtt_server.py" ]