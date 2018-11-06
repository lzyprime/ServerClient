# 服务器与客户端

## 服务器

> 因为想显示日志和输出，所以守护进程作为可选项在 `init.c` 中。

### 服务器启动后，接受客户端请求，并有最大限制，若满员，放弃链接。记录当前连接数。

### 服务器输入 `quit` 退出，其他内容则广播。下线前通知所有客户端。

### 接收客户端消息，实现登录或注册的认证。然后收到的内容转发其他用户

### 监听客户端发出的查看在线人数的请求。接收客户端下线通知，广播给其他用户。

### 有效信息存日志文件 `socket.log`

## 客户端

> 连接到服务器后，开始所有操作，否则直接退出

### 选择模式：登录， 注册，然后输入相关信息，正确则加入聊天

### `quit` 退出， `count` 请求在线人数
