
```cpp
enum class tpacket_type : uint8_t
{
	tpacket_none = 0,
	
	// connection state
	conn_req,
	conn_ack,
	conn_req_files,
	conn_ack_files,
	conn_req_file,
	conn_file_fragment,
	conn_ack_file,
	сonn_cancel,
	conn_error,
	
	// main game
	cl_snapshot,
	cl_req,
	cl_ack,
	
	sv_snapshot,
	sv_req,
	sv_ack,
	
	// disconnection
	disconnect_req,
	disconnect_ack,
	
	// status
	status_req,
	status_ack,
};
```

```cpp
struct tpacket_header
{
	uint64_t timestamp;
	uint64_t id;
	tpacket_type type;
};
```

```cpp
struct tpacket_conn_req
{
	
};
```

```cpp
struct tpacket_conn_ack
{
	uint8_t reject;
	char reject_reason[MAX_PROTOCOL_REASON_SIZE];
};
```

```cpp
struct tpacket_conn_req_files
{
	int16_t files_count;
	char filenames[MAX_PROTOCOL_FILES][MAX_PROTOCOL_FILENAME_SIZE];
};
```

```cpp
struct tpacket_conn_ack_files
{
	
};
```
