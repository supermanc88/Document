# FWPS_INCOMING_VALUES0

## 结构

```C++
typedef struct FWPS_INCOMING_VALUES0_ {
  UINT16               layerId;
  UINT32               valueCount;
  FWPS_INCOMING_VALUE0 *incomingValue;
} FWPS_INCOMING_VALUES0;
```

**layerId**: 指向的是过滤分层标识

**valueCount**: 表示下面`FWPS_INCOMING_VALUE0`数组元素大小。

**incomingValue**: 具体的包含什么数据，查看[官方解释](https://docs.microsoft.com/en-us/windows-hardware/drivers/network/data-field-identifiers)

```C++
typedef struct FWP_VALUE0_ {
  FWP_DATA_TYPE type;
  union {
    UINT8                 uint8;
    UINT16                uint16;
    UINT32                uint32;
    UINT64                *uint64;
    INT8                  int8;
    INT16                 int16;
    INT32                 int32;
    INT64                 *int64;
    float                 float32;
    double                *double64;
    FWP_BYTE_ARRAY16      *byteArray16;
    FWP_BYTE_BLOB         *byteBlob;
    SID                   *sid;
    FWP_BYTE_BLOB         *sd;
    FWP_TOKEN_INFORMATION *tokenInformation;
    FWP_BYTE_BLOB         *tokenAccessInformation;
    LPWSTR                unicodeString;
    FWP_BYTE_ARRAY6       *byteArray6;
  };
} FWP_VALUE0;
```