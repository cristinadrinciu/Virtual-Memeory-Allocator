
~Virtual Memory Allocator~

In this program I tried to recreate a simulator of a virtual Memory
Allocator. 
The program is splitted in 3 files: vma, commands and main.
Each file has it's own header file.

In vma.h are the Macros, the structures and the functions used. 
The macro DIE is used for the deffensive programming, given in the labs.
In vma.c they are all expanded.

First of all, in the code I gave introduced the functions used in the labs for double linked list.

Dll_create creates a generic double linked list, free_list frees it, dll_add_nt_node and dll_remove_nth_node inserts/removes a node in the list.

Alloc_arena creates the arena, it mallocs the arena as a structure, atributes the size and creates the linked list of the blocks.

Dealloc_arena frees all the used resorces: it frees the rw_buffer of each miniblock, then the free_list is used to free each list of miniblock_list of each block, then free_list is used to free the list of blocks and at the end, the arena is freed asa a data structure.

Find_block is a function that finds the miniblock of the given address, either if it is the start_address, or is in the interior od the miniblock.

I have creted some functions that are used once in the alloc_block functions, just to crete some space.

Error_alloc_block displays all the error messages of this operation. It returns and int so that if the function returns 0, an error is activated so in alloc_block the process is stoped by return.

Connect_blocks is a function that connects a block with its next, converting them in miniblocks. First the miniblock lists are connected, the size of the miniblock lists increses, the block is resised, an then the block from the right is removed from the alloc_list.
Due to an inconvenience of the index and of my approach, adding a block in interior was made "manually", without calling the function add_node.

Alloc_block allocs a block in the arena and it treats the case in which the blocks are tangent. 
First, it adds the block normally in the alloc_list, using the add_node function, by createing the miniblock which is given as parametre of "void *data". All the cases are treated in the function insert_node.
After that, the 3 cases of tangence are treated: when the new block is tangent with its prev, next or both.
In each case, the function connect_blocks is called, working in each case with the block "from the left", with the prev or with the new_block.

Free_block frees a block from the arena->alloc_list. First, the error cases are treated. Then we find the miniblock tha we have to free.
After that I treated different cases: when the arena has only one block, when the block has only one miniblock, when it is at the end, at the beggining and in interior.
For that I used the call of the function remove_node, needing before that an index.
Fpr the interior case, I made the modifications for spliting the block in two: create a new block, associate the miniblock list for the new block and adding it in the arena alloc_list with add_node function.

The function mprotect changes the permissions for the miniblocks. For that I called the function find_block to search the miniblock by the given address. First of all were treated the error cases.

For read and write operations I have made another two functions verif_perm_read and verif_perm_write.
Because of the mprotect function, is needed to verify if we have a liniar space for read/write.
Which means that I have to check if there are enough miniblocks with the right permission to read/write the number of bytes given. 
If they are not enough in a block or if there is not a conituous number of miniblocks, the operations failes. If there is a linear number of miniblocks, but not enoigh for the size, we calculate with the function number_bytes how much we read/write.

For both read and write, we find the miniblock of the given address. 
We treat the error cases and then I treat the easies case, when the size is just enough for one miniblock. In other case, we have to write/read the information on multiple miniblocks.
For read is also the condition to not display a non-printable character ('\0').

There are other functions like used_memory and number_of_miniblocks that helps for pmap operation in printing the info about the arena. 
Also the function perm_print that prints the permissios of the octal number given as a parametre (0 - 8).

In pmap is printed all the info of the arena, most of the data printed in hexadecimal format (%lX).

In the file commands are the calling of each operation, which includes reading the parametres needed, lise size, address, etc.

For calling the write function is a different approach for the data that needs to be written in the miniblocks. 
We only need size bytes from the text that is read as a parametre. So we build the text that we need with size bytes as dimention. 
Because there possibilaties to be read ENTER ('\n'), we read line with line, until we get the needed number of bytes. We get as a parametre for the function write the text/data with the right number of bytes, not the whole read text.

For calling mprotect the "code" given as a parametre of the command is converted in octal, so in calling mprotect the parametre is already a number.

In main is just the command which is read and the arena is declared. When we read the command DEALLOC_ARENA,the program is finished. Until then, we call the right operation of the given command. 

I made this approach of a different file with the commands for an airy and more readable main.
