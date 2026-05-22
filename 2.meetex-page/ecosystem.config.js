module.exports = {
  apps: [
    {
      name: "meetex",
      script: "server.js",
      instances: 1,
      exec_mode: "cluster",
      out_file: "/var/log/meetex/meetex.log", // 标准输出日志文件路径
      error_file: "/var/log/meetex/error.log", // 错误输出日志文件路径
      log_date_format: "YYYY-MM-DD HH:mm Z",
    },
  ],
};
