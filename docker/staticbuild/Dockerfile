FROM emscripten/emsdk:3.1.15 AS builder
RUN apt-get update && apt-get install -y libglm-dev inotify-tools libssl-dev libz-dev
RUN embuilder.py build zlib libpng
WORKDIR /project
COPY . .
RUN make VERSION=${VERSION:-unk} DISABLE_AUTO_REFRESH=1 -j $(nproc) -l $(nproc) ${MAKEOPTS:-all}

FROM nginx:stable-alpine
RUN sed -i -E 's,worker_processes.+auto;,worker_processes 1;,g' /etc/nginx/nginx.conf
COPY --from=builder /project/out/ /var/www/ourworldofpixels.com/
COPY --from=builder /project/src/ /var/www/projectsrc/
COPY ./config/nginx.conf /etc/nginx/conf.d/default.conf
RUN sed -i -E 's,^.+# del_on_static$,return 404;,g' /etc/nginx/conf.d/default.conf
