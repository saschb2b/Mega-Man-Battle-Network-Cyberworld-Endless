FROM python:3.12-slim AS build
WORKDIR /workspace
COPY server/ server/
COPY shared/online/ shared/online/
COPY LICENSE LICENSE
RUN python3 server/build.py --output /dist/cyberworld-server.pyz

FROM python:3.12-slim
RUN useradd --system --uid 10001 --no-create-home cyberworld
# (/backups: an empty named volume mounted there takes this owner, for
# deploy/coolify/compose.yaml's backup service)
RUN mkdir -p /data /backups && chown 10001:10001 /data /backups && chmod 700 /data /backups
COPY --from=build /dist/ /app/
USER 10001
EXPOSE 8765
ENTRYPOINT ["python3", "/app/cyberworld-server.pyz"]
CMD ["--host", "0.0.0.0", "--state-dir", "/data"]
