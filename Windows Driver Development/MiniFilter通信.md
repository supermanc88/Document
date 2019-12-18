# MiniFilter 通信

Minifilter支持通过端口的用户模式和内核模式之间的通信。端口通信不进行缓冲，所以它更快更有效。

## MiniFilter中创建通信服务器端口

```
NTSTATUS FLTAPI FltCreateCommunicationPort(
  PFLT_FILTER            Filter,
  PFLT_PORT              *ServerPort,
  POBJECT_ATTRIBUTES     ObjectAttributes,
  PVOID                  ServerPortCookie,
  PFLT_CONNECT_NOTIFY    ConnectNotifyCallback,
  PFLT_DISCONNECT_NOTIFY DisconnectNotifyCallback,
  PFLT_MESSAGE_NOTIFY    MessageNotifyCallback,
  LONG                   MaxConnections
);
```

此函数创建一个可接收从用户模式的连接请求的通信服务器端口。

当用户模式调用者尝试连接到端口时，MiniFilter会调用`ConnectNotifyCallback`函数创建新的连接。


**当使用`FltCloseCommunicationPort`关闭服务器端口时，任何新的连接都是不允许的，但是，现有的连接仍然保持连接，直到它们被用户模式应用程序或MiniFilter关闭，或MiniFilter驱动卸载。**

### 参数

- Filter

MiniFilter的句柄

- ServerPort

用来返回用于通信的服务器端口。MiniFilter使用该句柄来监听来自用户模式的连接请求

- ObjectAttributes

使用`InitializeObjectAttributes`初始化

- ServerPortCookie

一般为NULL

- ConnectNotifyCallback

每当一个用户模式应用程序调用`FilterConnectCommunicationPort`发送连接请求时，MiniFilter会调用这个回调函数。不可为NULL

- DisconnectNotifyCallback

当客户端连接数达到0时，或MiniFilter要卸载时，MiniFilter会调用此回调函数。不可为NULL

- MessageNotifyCallback

每当用户模式应用程序调用`FilterSendMessage`将消息发送到MiniFilter时，MiniFilter会调用此函数。可为NULL

- MaxConnections

允许客户端同时连接这个服务器端口的最大数目。此参数必需的，且必须大于0.

## 用户模式连接端口

```
HRESULT FilterConnectCommunicationPort(
  LPCWSTR               lpPortName,
  DWORD                 dwOptions,
  LPCVOID               lpContext,
  WORD                  wSizeOfContext,
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  HANDLE                *hPort
);
```

### 参数

- lpPortName

指向包含通信服务器端口的字符串，如：`L"\\MyFilterPort"`

- dwOptions

一般为0

- lpContext

一般为NULL

- wSizeOfContext

一般为NULL

- lpSecurityAttributes

一般为NULL

- hPort

用来返回新创建的连接的句柄



## 通信

### MiniFilter框架中

#### 收消息

当用户模式发送消息时，MiniFilter中`MessageNotifyCallback`回调函数会收到消息。

#### 发消息

`FltSendMessage`向一个**正在等待**消息的用户模式应用程序发送消息

```
NTSTATUS FLTAPI FltSendMessage(
  PFLT_FILTER    Filter,
  PFLT_PORT      *ClientPort,
  PVOID          SenderBuffer,
  ULONG          SenderBufferLength,
  PVOID          ReplyBuffer,
  PULONG         ReplyLength,
  PLARGE_INTEGER Timeout
);
```

参数`ClientPort`从`ConnectNotifyCallback`中获取

**`Timeout`表示等待时间，直到用户模式应用程序接收到消息并且返回一个回复,如果设置为NULL，则无限制等待。当`ReplyBuffer`和`Timeout`为空时，应用层用户程序接收到消息后，`FltSendMessage`会立刻返回`STATUS_SUCCESS`。**

**当`ReplyBuffer`不为空时，应用层使用`FilterGetMessage`接收到消息后，应用层用户程序必须使用`FilterReplyMessage`向Minifilter回复一个信息，当`FltSendMessage`接收到这个信息后，函数返回`STATUS_SUCCESS`。**

### 应用层用户程序

#### 收消息

使用`FilterGetMessage`和`FilterReplyMessage`函数

```
HRESULT FilterGetMessage(
  HANDLE                 hPort,
  PFILTER_MESSAGE_HEADER lpMessageBuffer,
  DWORD                  dwMessageBufferSize,
  LPOVERLAPPED           lpOverlapped
);
```

```
HRESULT FilterReplyMessage(
  HANDLE               hPort,
  PFILTER_REPLY_HEADER lpReplyBuffer,
  DWORD                dwReplyBufferSize
);
```

**注意：`FilterGetMessage`的buffer结构体中需使用`FILTER_MESSAGE_HEADER`结构，`FilterReplyMessage`的buffer结构体中需使用`FILTER_REPLY_HEADER`结构。何时需要reply见上文。**


#### 发消息

```
HRESULT FilterSendMessage(
  HANDLE  hPort,
  LPVOID  lpInBuffer,
  DWORD   dwInBufferSize,
  LPVOID  lpOutBuffer,
  DWORD   dwOutBufferSize,
  LPDWORD lpBytesReturned
);
```

使用此函数发消息时，MiniFilter中接收消息的为`MessageNotifyCallback`函数


## 关闭

MiniFilter中使用`FltCloseClientPort`函数，每个连入Minifilter的应用层通信可在`ConnectNotifyCallback`函数中获得端口。

应用层用户程序使用`CloseHandle`函数。