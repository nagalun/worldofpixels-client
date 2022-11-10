FROM emscripten/emsdk:3.1.23
RUN apt-get update && apt-get install -y libglm-dev inotify-tools libssl-dev libz-dev
RUN mkdir -p /wsockifier && chown emscripten:emscripten /wsockifier
USER emscripten
RUN embuilder.py build zlib libpng
WORKDIR /wsockifier
RUN git clone --recurse-submodules https://github.com/nagalun/websockifier . \
        && make -j3
USER root
RUN cp -v ./out /usr/local/bin/websockifier
USER emscripten
COPY watcher.sh /watcher.sh
WORKDIR /project
ENTRYPOINT ["/bin/bash", "/watcher.sh"]
