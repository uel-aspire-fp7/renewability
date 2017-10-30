FROM ubuntu:16.04
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y build-essential libmysqlclient-dev libwebsockets-dev

# Build the ASCL
COPY modules/ascl /opt/ASCL
RUN /opt/ASCL/build.sh

# Build manager
COPY modules/renewability /tmp/renewability
RUN /tmp/renewability/build_server.sh /opt/renewability /opt/ASCL

# Clean up
RUN rm -rf /tmp

ENTRYPOINT /opt/renewability/renewability_manager >> /opt/online_backends/renewability/manager-out.log 2>&1
