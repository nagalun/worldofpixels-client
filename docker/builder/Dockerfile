FROM emscripten/emsdk:3.1.20
RUN apt-get update && apt-get install -y libglm-dev inotify-tools libssl-dev libz-dev
RUN embuilder.py build zlib libpng
WORKDIR /wsockifier
RUN git clone --recurse-submodules https://github.com/nagalun/websockifier . \
        && make -j3 && cp -v ./out /usr/local/bin/websockifier
COPY watcher.sh /watcher.sh
WORKDIR /project
ENTRYPOINT ["/bin/bash", "/watcher.sh"]
