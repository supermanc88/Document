# ELF文件格式



ELF (Executable and Linkable Format)是一种为**可执行文件，目标文件，共享链接库和内核转储**(core dumps)准备的标准文件格式。Linux和很多类Unix操作系统都使用这个格式。



一个ELF文件由以下三部分组成：

- ELF头(ELF header) - 描述文件的主要特性：类型，CPU架构，入口地址，现有部分的大小和偏移等等；
- 程序头表(Program header table) - 列举了所有有效的段(segments)和他们的属性。 程序头表需要加载器将文件中的节加载到虚拟内存段中；
- 节头表(Section header table) - 包含对节(sections)的描述



## ELF头(ELF header)

ELF头(ELF header)位于文件的开始位置。 它的主要目的是**定位文件的其他部分**。 文件头主要包含以下字段：

- ELF文件鉴定 - 一个字节数组用来确认文件是否是一个ELF文件，并且提供普通文件特征的信息；
- 文件类型 - 确定文件类型。 这个字段描述文件是一个重定位文件，或可执行文件,或...；
- 目标结构；
- ELF文件格式的版本；
- 程序入口地址；
- 程序头表的文件偏移；
- 节头表的文件偏移；
- ELF头(ELF header)的大小；
- 程序头表的表项大小；
- 其他字段...



```c
typedef struct elf64_hdr {
  unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" */
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;		/* Entry point virtual address */
  Elf64_Off e_phoff;		/* Program header table file offset */
  Elf64_Off e_shoff;		/* Section header table file offset */
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;
```



## 程序头表(Program header table)

```c
typedef struct elf64_phdr {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;		/* Segment file offset */
  Elf64_Addr p_vaddr;		/* Segment virtual address */
  Elf64_Addr p_paddr;		/* Segment physical address */
  Elf64_Xword p_filesz;		/* Segment size in file */
  Elf64_Xword p_memsz;		/* Segment size in memory */
  Elf64_Xword p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;
```







## 节(sections)



所有的数据都存储在ELF文件的节(sections)中。 我们通过节头表中的索引(index)来确认节(sections)。 节头表表项包含以下字段：

- 节的名字；
- 节的类型；
- 节的属性；
- 内存地址；
- 文件中的偏移；
- 节的大小；
- 到其他节的链接；
- 各种各样的信息；
- 地址对齐；
- 这个表项的大小，如果有的话；



```c
typedef struct elf64_shdr {
  Elf64_Word sh_name;		/* Section name, index in string tbl */
  Elf64_Word sh_type;		/* Type of section */
  Elf64_Xword sh_flags;		/* Miscellaneous section attributes */
  Elf64_Addr sh_addr;		/* Section virtual addr at execution */
  Elf64_Off sh_offset;		/* Section file offset */
  Elf64_Xword sh_size;		/* Size of section in bytes */
  Elf64_Word sh_link;		/* Index of another section */
  Elf64_Word sh_info;		/* Additional section information */
  Elf64_Xword sh_addralign;	/* Section alignment */
  Elf64_Xword sh_entsize;	/* Entry size if section holds table */
} Elf64_Shdr;
```



## vmlinux

`vmlinux` 也是一个可重定位的ELF文件。 我们可以使用 `readelf` 工具来查看它。 首先，让我们看一下它的头部：

readelf help

```shell
[root@CentOS6 2.6.32-754.el6.x86_64]# readelf 
Usage: readelf <option(s)> elf-file(s)
 Display information about the contents of ELF format files
 Options are:
  -a --all               Equivalent to: -h -l -S -s -r -d -V -A -I
  -h --file-header       Display the ELF file header
  -l --program-headers   Display the program headers
     --segments          An alias for --program-headers
  -S --section-headers   Display the sections' header
     --sections          An alias for --section-headers
  -g --section-groups    Display the section groups
  -t --section-details   Display the section details
  -e --headers           Equivalent to: -h -l -S
  -s --syms              Display the symbol table
      --symbols          An alias for --syms
  -n --notes             Display the core notes (if present)
  -r --relocs            Display the relocations (if present)
  -u --unwind            Display the unwind info (if present)
  -d --dynamic           Display the dynamic section (if present)
  -V --version-info      Display the version sections (if present)
  -A --arch-specific     Display architecture specific information (if any).
  -c --archive-index     Display the symbol/file index in an archive
  -D --use-dynamic       Use the dynamic section info when displaying symbols
  -x --hex-dump=<number|name>
                         Dump the contents of section <number|name> as bytes
  -p --string-dump=<number|name>
                         Dump the contents of section <number|name> as strings
  -R --relocated-dump=<number|name>
                         Dump the contents of section <number|name> as relocated bytes
  -w[lLiaprmfFsoR] or
  --debug-dump[=rawline,=decodedline,=info,=abbrev,=pubnames,=aranges,=macro,=frames,=str,=loc,=Ranges]
                         Display the contents of DWARF2 debug sections
  -I --histogram         Display histogram of bucket list lengths
  -W --wide              Allow output width to exceed 80 characters
  @<file>                Read options from <file>
  -H --help              Display this information
  -v --version           Display the version number of readelf
```



```shell
[root@CentOS6 2.6.32-754.el6.x86_64]# readelf -h vmlinux 
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x1000000
  Start of program headers:          64 (bytes into file)
  Start of section headers:          134752544 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         6
  Size of section headers:           64 (bytes)
  Number of section headers:         50
  Section header string table index: 47
```





下面是根据浅显的学习，写的从data段中读取全局变量的代码

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>

char master_key[16] = "0123456789abcdef";

int main(int argc, char *argv[]) {
    FILE *fp;
    Elf64_Ehdr elf_header;
    Elf64_Shdr *p_elf_section_header;
    char *p_section_str = NULL;
    int i = 0;
    char *section_name = NULL;

    fp = fopen("./master_key", "r");
    if (fp == NULL)
        exit(-1);

    fread(&elf_header, sizeof(elf_header), 1, fp);


    printf("e_shoff = %ld, e_shentsize = %d, e_shnum = %d, e_shstrndx = %d\n", elf_header.e_shoff, elf_header.e_shentsize, elf_header.e_shnum, elf_header.e_shstrndx);

    p_elf_section_header = malloc(sizeof(Elf64_Shdr) * elf_header.e_shnum);
    fseek(fp, elf_header.e_shoff, SEEK_SET);
    fread(p_elf_section_header, sizeof(Elf64_Shdr) * elf_header.e_shnum, 1, fp);


    p_section_str = malloc(p_elf_section_header[elf_header.e_shstrndx].sh_size);
    fseek(fp, p_elf_section_header[elf_header.e_shstrndx].sh_offset, SEEK_SET);
    fread(p_section_str, p_elf_section_header[elf_header.e_shstrndx].sh_size, 1, fp);


    for (i = 0; i < elf_header.e_shnum; i++) {
        printf("sh_name = %d\n", p_elf_section_header[i].sh_name);
        section_name = p_section_str + p_elf_section_header[i].sh_name;

        printf("section name = %s\n", section_name);

        if (strcmp(section_name, ".data") == 0) {
            break;
        }
    }

    Elf64_Off data_offset = p_elf_section_header[i].sh_offset;
    Elf64_Xword data_size = p_elf_section_header[i].sh_size;

    char * data_content = malloc(data_size);
    fseek(fp, data_offset, SEEK_SET);
    fread(data_content, data_size, 1, fp);

    int j = 0;
    for (j = 16; j < data_size; j++) {
        printf("%02x ", data_content[j]);
    }
    printf("\n");

    free(data_content);
    free(p_section_str);
    free(p_elf_section_header);

    fclose(fp);

    printf("Hello, World!\n");

    return 0;
}

```

