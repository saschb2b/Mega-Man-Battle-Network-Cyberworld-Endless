FROM python:3.12-slim AS build
WORKDIR /workspace
COPY server/ server/
COPY shared/online/ shared/online/
COPY LICENSE LICENSE
RUN python3 server/build.py --output /dist/cyberworld-server.pyz

FROM python:3.12-slim
RUN useradd --system --uid 10001 --no-create-home cyberworld
RUN mkdir -p /data && chown 10001:10001 /data && chmod 700 /data
COPY --from=build /dist/ /app/
USER 10001
EXPOSE 8765
ENTRYPOINT ["python3", "/app/cyberworld-server.pyz"]
CMD ["--host", "0.0.0.0", "--state-dir", "/data"]
