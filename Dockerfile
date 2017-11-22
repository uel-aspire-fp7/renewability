FROM ubuntu:16.04
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y build-essential libmysqlclient-dev libwebsockets-dev mysql-client netcat wget

# Build the ASCL
COPY modules/ascl /opt/ASCL
RUN /opt/ASCL/build.sh

# Build manager
COPY modules/renewability /tmp/renewability
RUN /tmp/renewability/build_server.sh /opt/renewability /opt/ASCL
COPY modules/renewability/scripts /opt/renewability/scripts

# Clean up
RUN rm -rf /tmp/*

# Slight hack: make sure mysql is running before we actually start renewability_manager
ENTRYPOINT while ! nc -z mysql 3306; do sleep 1; done; /opt/renewability/renewability_manager >> /opt/online_backends/renewability/manager-out.log 2>&1
