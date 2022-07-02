#include "patcher9x.h"

typedef struct _pfiles_t
{
	const char *file;
	const char *dir;
	uint32_t flags;
} pfiles_t;


static pfiles_t pfiles[] = {
	{"VMM32.VXD",    "",      PATCH_VX_PACK | PATCH_VMM_ALL | PATCH_CPU_SPEED_ALL },
	{"VMM.VXD",      "VMM32", PATCH_VX_UNPACK | PATCH_VMM_ALL },
	{"NTKERN.VXD",   "VMM32", PATCH_VX_UNPACK | PATCH_CPU_SPEED_ALL },
	{"IOS.VXD",      "VMM32", PATCH_VX_UNPACK | PATCH_CPU_SPEED_ALL },
	{"ESDI_506.PDR", "IOSUB", PATCH_CPU_SPEED_ALL },
	{"SCSIPORT.PDR", "IOSUB", PATCH_CPU_SPEED_ALL },
	{"NDIS.VXD",     "",      PATCH_CPU_SPEED_NDIS_ALL },
	{NULL,           NULL,    0 }
};

#define L_F_CREATED 1
#define L_T_CREATED 2

typedef struct _pmodfile_t
{
	char *tname;
	char *fname;
	uint32_t applied;
	uint32_t exists;
	uint32_t flags;
	struct _pmodfile_t *next;
} pmodfile_t;

struct _pmodfiles_t
{
	pmodfile_t *first;
	pmodfile_t *last;
	size_t      length;
};

static void pmodfile_free(pmodfile_t *item)
{
	if(item->tname != NULL)
	{
		fs_path_free(item->tname);
	}
	
	if(item->fname != NULL)
	{
		fs_path_free(item->fname);
	}
	
	free(item);
}

static pmodfiles_t pmodfiles_init()
{
	pmodfiles_t list = (pmodfiles_t)malloc(sizeof(struct _pmodfiles_t));
	if(list != NULL)
	{
		list->first   = NULL;
		list->last    = NULL;
		list->length  = 0;
	}
	
	return list;
}

static void pmodfiles_add(pmodfiles_t list, char *fname, char *tname, uint32_t flags, uint32_t applied, uint32_t exists)
{
	pmodfile_t *item = (pmodfile_t*)malloc(sizeof(pmodfile_t));
	if(item != NULL)
	{
		item->fname = fname;
		item->tname = tname;
		item->flags = flags;
		
		item->exists  = exists;
		item->applied = applied;
		
		item->next  = NULL;
		
		if(list->first == NULL)
		{
			list->first = item;
			list->last  = item;
		}
		else
		{
			list->last->next = item;
			list->last = item;
		}
	}
}

#define FLAGS() ((pfile->flags | global_flags) & (~global_unmask))

pmodfiles_t files_lookup(const char *upath, uint32_t global_flags, uint32_t global_unmask, uint32_t lookup_flags)
{
	char *ename, *vname;
	pfiles_t *pfile;
	pmodfiles_t list;
	char *path = (char*)upath;
	char *path_mem = NULL;
	
	list = pmodfiles_init();
	if(!list)
	{
		return NULL;
	}
	
	if(!fs_is_dir(upath))
	{
		path_mem = fs_dirname(upath);
		if(path_mem != NULL)
		{
			path = path_mem;
		}
	}
	
	ename = fs_path_get(path, "VMM32.W3", NULL);
	vname = fs_path_get(path, "VMM32.VXD", NULL);
	
	for(pfile = pfiles; pfile->file != NULL; pfile++)
	{
		char *dir = NULL, *dir_mem = NULL;
		char *fname = NULL, *tname = NULL;
		uint32_t exists;
		uint32_t applied;
		int list_item_added = 0;
		
		if(strlen(pfile->dir) > 0 && (lookup_flags & PATCH_LOOKUP_CABS) == 0 && (lookup_flags & PATCH_LOOKUP_ONE_CAB) == 0)
		{
			dir_mem = dir = fs_path_get(path, pfile->dir, NULL);
		}
		else
		{
			dir = path;
		}
		
		fname = fs_path_get(dir,   pfile->file, NULL);
		tname = fs_path_get(dir,   pfile->file, "p9x");
		
		//printf("fname: %s\n", fname);
		
		if(fs_file_exists(fname))
		{
			FILE *fp;
			
			if((pfile->flags & PATCH_VX_PACK) != 0)
			{
				if((lookup_flags & PATCH_LOOKUP_NO_VMM32) != 0)
				{
					continue;
				}
				
				if(wx_to_w3(fname, ename) != PATCH_OK)
				{
					continue;
				}
				
				fp = fopen(ename, "rb");
			}
			else
			{
				fp = fopen(fname, "rb");
			}
			
			if(fp)
			{
				patch_selected(fp, tname, FLAGS(), &applied, &exists);
				fclose(fp);
				
				pmodfiles_add(list, fname, tname, L_T_CREATED, applied, exists);
				list_item_added = 1;
			}
		}
		else if((pfile->flags & PATCH_VX_UNPACK) != 0 && (lookup_flags & PATCH_LOOKUP_EXTRACTWX) != 0)
		{
			//printf("wx unpack: %s\n", pfile->file);
			if(wx_unpack(vname, pfile->file, fname, ename) == PATCH_OK)
			{
				FILE *fp = fopen(fname, "rb");
				if(fp)
				{
					patch_selected(fp, tname, FLAGS(), &applied, &exists);
					fclose(fp);
					
					pmodfiles_add(list, fname, tname, L_F_CREATED|L_T_CREATED, applied, exists);
					list_item_added = 1;
				}
			}
		}
		else if((lookup_flags & PATCH_LOOKUP_CABS) != 0)
		{
			if(cab_search_unpack(path, pfile->file, fname) > 0)
			{
				FILE *fp = fopen(fname, "rb");
				if(fp)
				{
					patch_selected(fp, tname, FLAGS(), &applied, &exists);
					fclose(fp);
					
					pmodfiles_add(list, fname, tname, L_F_CREATED|L_T_CREATED, applied, exists);
					list_item_added = 1;
				}
			}
		}
		else if((lookup_flags & PATCH_LOOKUP_ONE_CAB) != 0)
		{
			//printf("CAB unpack: %s -> %s\n", upath, fname);
			if(cab_unpack(upath, pfile->file, fname) > 0)
			{
				FILE *fp = fopen(fname, "rb");
				if(fp)
				{
					patch_selected(fp, tname, FLAGS(), &applied, &exists);
					fclose(fp);
					
					pmodfiles_add(list, fname, tname, L_F_CREATED|L_T_CREATED, applied, exists);
					list_item_added = 1;
				}
			}
		}
		
		if(list_item_added == 0)
		{
			if(tname != NULL) fs_path_free(tname);
			if(fname != NULL) fs_path_free(fname);
		}
		
		if(dir_mem != NULL) fs_path_free(dir_mem);
	} // for
	
	if(ename != NULL) fs_path_free(ename);
	if(vname != NULL) fs_path_free(vname);
	if(path_mem != NULL) fs_path_free(path_mem);
	
	return list;
}

/*
 C = file created
 E = exists patches
 N = new patches
*/

int files_status(pmodfiles_t list)
{
	int files_count = 0;	
	pmodfile_t *pmod;
	
	printf("------------------------------------------------------------\n");
	printf(" Status | File path\n");
	printf("--------+---------------------------------------------------\n");
	
	for(pmod = list->first; pmod != NULL; pmod = pmod->next)
	{
		char c_created = ' ';
		char c_applied = ' ';
		char c_exists  = ' ';
		
		if(pmod->applied != 0)
		{
			files_count++;
			c_applied = 'N';
		}
		if(pmod->exists  != 0)  c_exists = 'E';
		if((pmod->flags & L_F_CREATED) != 0)  c_created = 'C';
		
		printf("  %c %c %c | %s\n", c_created, c_applied, c_exists, pmod->fname);		
	}
	
	printf("------------------------------------------------------------\n");
	printf("  N - new patches to apply\n");
	printf("  C - file created\n");
	printf("  E - some already patches\n");
	printf("------------------------------------------------------------\n");
	
	if(files_count > 0)
	{
		printf("  Status: new patches to apply: %d\n", files_count);
	}
	else
	{
		printf("  Status: no patches to apply found\n");
	}
	
	return files_count;
}

int files_commit(pmodfiles_t *plist)
{
	pmodfile_t *pmod, *pmod_clean;
	pmodfiles_t list = *plist;
	int cnt = 0;
	
	for(pmod = list->first; pmod != NULL; )
	{
		if(pmod->applied != 0)
		{
			patch_backup_file(pmod->fname, 0);
			if(fs_file_exists(pmod->tname))
			{
				fs_rename(pmod->tname, pmod->fname);
				cnt++;
			}
		}
		else
		{
			if(pmod->flags & L_T_CREATED)
			{
				fs_unlink(pmod->tname);
			}
			
			if(pmod->flags & L_F_CREATED)
			{
				fs_unlink(pmod->fname);
			}
		}
		
		pmod_clean = pmod;
		pmod = pmod->next;
		
		pmodfile_free(pmod_clean);
	}
	
	free(list);
	*plist = NULL;
	
	return cnt;
}

void files_cleanup(pmodfiles_t *plist)
{
	pmodfile_t *pmod, *pmod_clean;
	pmodfiles_t list = *plist;
	
	for(pmod = list->first; pmod != NULL; )
	{
		if(pmod->flags & L_T_CREATED)
		{
			fs_unlink(pmod->tname);
		}
			
		if(pmod->flags & L_F_CREATED)
		{
			fs_unlink(pmod->fname);
		}
		
		pmod_clean = pmod;
		pmod = pmod->next;
		
		pmodfile_free(pmod_clean);
	}
	
	free(list);
	*plist = NULL;
}
