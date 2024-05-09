FROM python:3.11-slim

WORKDIR /app

COPY requirements.common.txt ./
RUN pip install --no-cache-dir -r requirements.common.txt

COPY plantbot/requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY data/ PlantAlertBot.py owm.py mongodb.py ./

CMD [ "python", "./PlantAlertBot.py" ]