# match_token函数解析

源码如下：

```c
/**
 * match_one: - Determines if a string matches a simple pattern
 * @s: the string to examine for presense of the pattern
 * @p: the string containing the pattern
 * @args: array of %MAX_OPT_ARGS &substring_t elements. Used to return match
 * locations.
 *
 * Description: Determines if the pattern @p is present in string @s. Can only
 * match extremely simple token=arg style patterns. If the pattern is found,
 * the location(s) of the arguments will be returned in the @args array.
 */
static int match_one(char *s, const char *p, substring_t args[])
{
    // 比如传进来的是 s： pid=1
    // p：pid=%d
	char *meta;
	int argc = 0;

	if (!p)
		return 1;

	while(1) {
		int len = -1;
		meta = strchr(p, '%');
		if (!meta)
			return strcmp(p, s) == 0;

		if (strncmp(p, s, meta-p))
			return 0;

        // 如果%之前的内容都一样的话
		s += meta - p;		// s定位到1的位置
		p = meta + 1;		// p定位到d的位置

		if (isdigit(*p))
			len = simple_strtoul(p, (char **) &p, 10);
		else if (*p == '%') {
			if (*s++ != '%')
				return 0;
			p++;
			continue;
		}
        // *p不是数字 也不是%

		if (argc >= MAX_OPT_ARGS)
			return 0;

		args[argc].from = s;
		switch (*p++) {				// 现在 *p是d
		case 's':
			if (strlen(s) == 0)
				return 0;
			else if (len == -1 || len > strlen(s))
				len = strlen(s);
			args[argc].to = s + len;
			break;
		case 'd':
			simple_strtol(s, &args[argc].to, 0);		// args[argc].to 返回NULL
			goto num;
		case 'u':
			simple_strtoul(s, &args[argc].to, 0);
			goto num;
		case 'o':
			simple_strtoul(s, &args[argc].to, 8);
			goto num;
		case 'x':
			simple_strtoul(s, &args[argc].to, 16);
		num:
			if (args[argc].to == args[argc].from)
				return 0;
			break;
		default:
			return 0;
		}
		s = args[argc].to;		// s = NULL
		argc++;
	}
}

/**
 * match_token: - Find a token (and optional args) in a string
 * @s: the string to examine for token/argument pairs
 * @table: match_table_t describing the set of allowed option tokens and the
 * arguments that may be associated with them. Must be terminated with a
 * &struct match_token whose pattern is set to the NULL pointer.
 * @args: array of %MAX_OPT_ARGS &substring_t elements. Used to return match
 * locations.
 *
 * Description: Detects which if any of a set of token strings has been passed
 * to it. Tokens can include up to MAX_OPT_ARGS instances of basic c-style
 * format identifiers which will be taken into account when matching the
 * tokens, and whose locations will be returned in the @args array.
 */
int match_token(char *s, const match_table_t table, substring_t args[])
{
	const struct match_token *p;

	for (p = table; !match_one(s, p->pattern, args) ; p++)
        // 看样子是 match_one 返回不为0时，循环中止
		;

	return p->token;
}
```



```c
/* associates an integer enumerator with a pattern string. */
struct match_token {
	int token;
	const char *pattern;
};

typedef struct match_token match_table_t[];

/* Maximum number of arguments that match_token will find in a pattern */
enum {MAX_OPT_ARGS = 3};

/* Describe the location within a string of a substring */
typedef struct {
	char *from;
	char *to;
} substring_t;
```



```c
static const match_table_t tokens = {
	{ecryptfs_opt_sig, "sig=%s"},
	{ecryptfs_opt_ecryptfs_sig, "ecryptfs_sig=%s"},
	{ecryptfs_opt_cipher, "cipher=%s"},
	{ecryptfs_opt_ecryptfs_cipher, "ecryptfs_cipher=%s"},
	{ecryptfs_opt_ecryptfs_key_bytes, "ecryptfs_key_bytes=%u"},
	{ecryptfs_opt_passthrough, "ecryptfs_passthrough"},
	{ecryptfs_opt_xattr_metadata, "ecryptfs_xattr_metadata"},
	{ecryptfs_opt_encrypted_view, "ecryptfs_encrypted_view"},
	{ecryptfs_opt_fnek_sig, "ecryptfs_fnek_sig=%s"},
	{ecryptfs_opt_fn_cipher, "ecryptfs_fn_cipher=%s"},
	{ecryptfs_opt_fn_cipher_key_bytes, "ecryptfs_fn_key_bytes=%u"},
	{ecryptfs_opt_unlink_sigs, "ecryptfs_unlink_sigs"},
	{ecryptfs_opt_check_dev_ruid, "ecryptfs_check_dev_ruid"},
	{ecryptfs_opt_err, NULL}
};
```

