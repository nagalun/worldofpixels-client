FROM emscripten/emsdk:3.1.23 AS builder
RUN apt-get update && apt-get install -y libglm-dev inotify-tools libssl-dev libz-dev
RUN mkdir -p /project && chown emscripten:emscripten /project
USER emscripten
RUN embuilder.py build zlib libpng
ARG VERSION=unk
WORKDIR /project
COPY . .
USER root
RUN chown -R emscripten:emscripten /project
USER emscripten
RUN make clean && make VERSION=${VERSION} DISABLE_AUTO_REFRESH=1 -j $(nproc) -l $(nproc) ${MAKEOPTS:-all}

FROM nginx:stable-alpine
RUN sed -i -E 's,worker_processes.+auto;,worker_processes 1;,g' /etc/nginx/nginx.conf
COPY --from=builder /project/out/ /var/www/ourworldofpixels.com/
COPY --from=builder /project/src/ /var/www/projectsrc/
COPY ./config/nginx.conf /etc/nginx/conf.d/default.conf
RUN sed -i -E 's,^.+# del_on_static$,return 404;,g' /etc/nginx/conf.d/default.conf
