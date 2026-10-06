FROM node:22-trixie-slim

# Sandcastle's agent, SV compiler, SDL3 development libraries, and Python checks.
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates clang curl dbus-daemon git gnome-keyring jq libclang-rt-dev libfreetype-dev libsecret-1-dev \
    libsdl3-dev libsdl3-ttf-dev make pkg-config python3 python3-venv \
    && rm -rf /var/lib/apt/lists/*

COPY tools/requirements-capabilities.txt /tmp/requirements-capabilities.txt
RUN python3 -m venv /opt/sv-venv \
    && /opt/sv-venv/bin/pip install --no-cache-dir -r /tmp/requirements-capabilities.txt \
    && rm /tmp/requirements-capabilities.txt
ENV PATH="/opt/sv-venv/bin:${PATH}"

RUN npm install -g @openai/codex

ARG AGENT_UID=1000
ARG AGENT_GID=1000
RUN groupmod -o -g "$AGENT_GID" node \
    && usermod -o -u "$AGENT_UID" -g "$AGENT_GID" -d /home/agent -m -l agent node

USER ${AGENT_UID}:${AGENT_GID}
WORKDIR /home/agent
ENTRYPOINT ["sleep", "infinity"]
