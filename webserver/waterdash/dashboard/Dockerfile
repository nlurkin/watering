FROM python:3.11-slim

WORKDIR /app

COPY requirements.common.txt ./
RUN pip install --no-cache-dir -r requirements.common.txt

COPY dashboard/requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY wsgi.py ./

CMD [ "/bin/bash" ]