# 环境变量配置说明

本项目的 Web 服务配置可通过环境变量覆盖，统一前缀为 `QQMUSIC_`。嵌套字段使用 `_` 分隔，例如 `server.host` -> `QQMUSIC_SERVER_HOST`。

注意：简单标量（字符串、数字、布尔）通过环境变量设置最可靠；复杂结构（映射、嵌套数组）建议直接修改 `web/config.toml` 或使用 `.env` 文件。

以下列出常用环境变量及其含义（中文）：

- **服务（server）**
  - `QQMUSIC_SERVER_HOST`：绑定地址，默认 `127.0.0.1`。
  - `QQMUSIC_SERVER_PORT`：监听端口，默认 `8080`。
  - `QQMUSIC_SERVER_WORKERS`：工作进程数（Uvicorn worker），默认 `1`。
  - `QQMUSIC_SERVER_LIMIT_CONCURRENCY`：每 worker 最大并发连接/任务（可选）。

- **日志（logging）**
  - `QQMUSIC_LOGGING_MODE`：日志模式，`console` / `file` / `both`。默认 `console`。
  - `QQMUSIC_LOGGING_LEVEL`：日志级别，`DEBUG` / `INFO` / `WARNING` / `ERROR` / `CRITICAL`。默认 `INFO`。
  - `QQMUSIC_LOGGING_FILE_PATH`：当 `mode` 包含 `file` 时的日志文件路径，默认 `web/data/logs/app.log`。

- **缓存（cache）**
  - `QQMUSIC_CACHE_TTL`：默认缓存过期时间（秒），默认 `60`。
  - `QQMUSIC_CACHE_MEMORY_MAX_SIZE`：内存缓存最大条目数，默认 `1024`。
  - `QQMUSIC_CACHE_BACKEND`：缓存后端，`memory` 或 `redis`（默认 `memory`）。
  - `QQMUSIC_CACHE_REDIS_URL`：当使用 `redis` 后端时的连接 URL（例如 `redis://localhost:6379/0`）。
  - `QQMUSIC_CACHE_REDIS_PREFIX`：Redis 键前缀（可选）。

- **凭据（credential）**
  - `QQMUSIC_CREDENTIAL_ENABLED`：是否启用全局默认登录凭证，`true`/`false`，默认 `false`。
  - `QQMUSIC_CREDENTIAL_API`：允许使用全局默认凭据的 API 映射（复杂结构，建议在 `config.toml` 中配置）。

- **凭据存储（credential.store）**
  - `QQMUSIC_CREDENTIAL_STORE_BACKEND`：状态存储后端（例如 `sqlite`），默认 `sqlite`。
  - `QQMUSIC_CREDENTIAL_STORE_PATH`：SQLite 文件路径（可选，默认在 `web/data` 下）。

- **安全（security）**
  - `QQMUSIC_SECURITY_ENABLED`：是否启用访问控制与限流，`true`/`false`，默认 `true`。
  - `QQMUSIC_SECURITY_IP_LIST_MODE`：IP 名单模式，`denylist` 或 `allowlist`（默认 `denylist`）。
  - `QQMUSIC_SECURITY_IP_ALLOWLIST`：白名单 IP 列表（逗号分隔或 JSON 数组，建议在 Toml 中配置）。
  - `QQMUSIC_SECURITY_IP_DENYLIST`：黑名单 IP 列表。
  - `QQMUSIC_SECURITY_TRUSTED_PROXY_IPS`：可信代理 IP/CIDR 列表（用于读取代理头）。
  - `QQMUSIC_SECURITY_CLIENT_IP_HEADER`：客户端真实 IP 头（例如 `x-forwarded-for`）。
  - `QQMUSIC_SECURITY_RATE_LIMIT_ENABLED`：是否启用 IP 维度限流，`true`/`false`。
  - `QQMUSIC_SECURITY_RATE_LIMIT_CAPACITY`：限流容量（次数），默认 `60`。
  - `QQMUSIC_SECURITY_RATE_LIMIT_WINDOW_SECONDS`：限流窗口（秒），默认 `60`。
  - `QQMUSIC_SECURITY_CONCURRENCY_LIMIT_ENABLED`：是否启用并发限制，`true`/`false`。
  - `QQMUSIC_SECURITY_CONCURRENCY_LIMIT`：单 worker 并发处理最大请求数。
  - `QQMUSIC_SECURITY_CONCURRENCY_RETRY_AFTER_SECONDS`：并发过载时返回的 `Retry-After` 秒数。

- **CORS（跨域）**
  - `QQMUSIC_CORS_ENABLED`：是否启用 CORS，`true`/`false`。
  - `QQMUSIC_CORS_ALLOW_ORIGINS`：允许跨域的 Origin 列表（逗号分隔或 JSON 数组）。
  - `QQMUSIC_CORS_ALLOW_CREDENTIALS`：是否允许携带凭据，`true`/`false`。
  - `QQMUSIC_CORS_ALLOW_METHODS`：允许的方法列表，例如 `GET,POST,OPTIONS`。
  - `QQMUSIC_CORS_ALLOW_HEADERS`：允许的请求头列表。
  - `QQMUSIC_CORS_MAX_AGE`：预检请求缓存秒数。

## 使用示例

在当前终端临时生效（只对本次运行有效）：

```bash
export QQMUSIC_SERVER_HOST=0.0.0.0
export QQMUSIC_SERVER_PORT=3000
QQMUSIC_LOGGING_MODE=file QQMUSIC_LOGGING_FILE_PATH=web/data/logs/app.log ./dist/QQMusicWeb/QQMusicWeb
```

或者在一行中只对单次命令生效：

```bash
QQMUSIC_SERVER_PORT=3000 ./dist/QQMusicWeb/QQMusicWeb
```

如果需要设置复杂的数组或映射，优先编辑 `web/config.toml`，或者在启动时加载 `.env` / 使用部署工具（如 Docker Compose）传入更复杂的结构。

---

文件位置：`web/config.example.toml` 中列出了完整的默认配置，建议将其复制为 `web/config.toml` 并按需调整，环境变量用于覆盖单个条目。
