# Optional reproducible Linux build environment for doomps1 (M0).
# Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
#
# Build the image (downloads PSn00bSDK release assets — needs network):
#   docker build -t doomps1-sdk .
# Build the engine:
#   docker run --rm -v "$PWD":/src -w /src doomps1-sdk make
#
# This does NOT run DuckStation or validate hardware. Host BIOS is required
# outside the container to play the resulting .cue/.bin.

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
	ca-certificates \
	curl \
	unzip \
	cmake \
	ninja-build \
	make \
	python3 \
	git \
	&& rm -rf /var/lib/apt/lists/*

ENV PSN00BSDK_ROOT=/opt/psn00bsdk
WORKDIR /opt

COPY scripts/setup-psn00bsdk.sh /tmp/setup-psn00bsdk.sh
RUN chmod +x /tmp/setup-psn00bsdk.sh \
	&& /tmp/setup-psn00bsdk.sh "${PSN00BSDK_ROOT}" \
	&& . "${PSN00BSDK_ROOT}/env.sh" \
	&& echo "PSN00BSDK_LIBS=${PSN00BSDK_LIBS}" \
	&& command -v mipsel-none-elf-gcc \
	&& mipsel-none-elf-gcc --version | head -n 1

# Persist env for interactive shells / make invocations.
RUN printf 'source /opt/psn00bsdk/env.sh\n' > /etc/profile.d/psn00bsdk.sh
ENV BASH_ENV=/opt/psn00bsdk/env.sh

WORKDIR /src
CMD ["bash", "-lc", "source /opt/psn00bsdk/env.sh && make"]
