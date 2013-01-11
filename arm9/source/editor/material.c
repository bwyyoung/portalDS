#include "editor/editor_main.h"

material_struct materials[NUMMATERIALS];
material_struct* defaultMaterial;
materialSlice_struct materialSlices[NUMMATERIALSLICES];
materialSlice_struct* defaultMaterialSlice;

void initMaterials(void)
{
	int i;
	for(i=0;i<NUMMATERIALS;i++)
	{
		material_struct* m=&materials[i];
		m->used=false;
		m->id=i;
		m->top=m->bottom=m->side=NULL;
	}
	for(i=0;i<NUMMATERIALSLICES;i++)
	{
		materialSlice_struct* ms=&materialSlices[i];
		ms->used=false;
		ms->id=i;
		ms->img=NULL;
	}
	defaultMaterial=createMaterial();
	defaultMaterialSlice=createMaterialSlice();
	loadMaterialSlice(defaultMaterialSlice,"default.pcx");
}

materialSlice_struct* createMaterialSlice()
{
	int i;
	for(i=0;i<NUMMATERIALSLICES;i++)
	{
		if(!materialSlices[i].used)
		{
			materialSlice_struct* ms=&materialSlices[i];
			ms->used=true;
			ms->img=NULL;
			return ms;
		}
	}
	return NULL;
}

material_struct* createMaterial()
{
	int i;
	for(i=0;i<NUMMATERIALS;i++)
	{
		if(!materials[i].used)
		{
			material_struct* m=&materials[i];
			m->used=true;
			m->top=m->bottom=m->side=NULL;
			return m;
		}
	}
	return NULL;
}

void loadMaterialSlice(materialSlice_struct* ms, char* filename)
{
	if(ms)
	{
		ms->img=createTexture(filename,"textures");
		ms->align=false;
		ms->factor=1;
	}
}

void loadMaterialSlices(char* filename)
{
	dictionary* dic=iniparser_load(filename);
	int i=0;
	char* r;
	char key[255];
	sprintf(key,"slice%d:texture",i);
	while((r=dictionary_get(dic, key, NULL)))
	{
		materialSlice_struct* ms=createMaterialSlice();
		loadMaterialSlice(ms,r);
		NOGBA("loaded %d : %s",i,r);
		
			int k=0;
			sprintf(key,"slice%d:align",i);
			sscanf(dictionary_get(dic, key, "0"),"%d",&k);
			ms->align=(k!=0);
			sprintf(key,"slice%d:factor",i);
			sscanf(dictionary_get(dic, key, "1"),"%d",&k);
			ms->factor=k;
		
		// if(!ms->img){break;}
		i++;
		sprintf(key,"slice%d:texture",i);		
	}
	iniparser_freedict(dic);
}

void loadMaterials(char* filename)
{
	dictionary* dic=iniparser_load(filename);
	int i=0;
	char *r1, *r2, *r3;
	char key1[255],key2[255],key3[255];
	sprintf(key1,"material%d:top",i);
	sprintf(key2,"material%d:side",i);
	sprintf(key3,"material%d:bottom",i);
	(r1=dictionary_get(dic, key1, NULL));(r2=dictionary_get(dic, key2, NULL));(r3=dictionary_get(dic, key3, NULL));
	while(r1||r2||r3)
	{
		material_struct* m=createMaterial();
		if(!m){break;}
		if(r1)
		{
			int k;
			sscanf(r1,"%d",&k);
			m->top=&materialSlices[k+1];
		}
		if(r2)
		{
			int k;
			sscanf(r2,"%d",&k);
			m->side=&materialSlices[k+1];
		}
		if(r3)
		{
			int k;
			sscanf(r3,"%d",&k);
			m->bottom=&materialSlices[k+1];
		}
		i++;
		sprintf(key1,"material%d:top",i);
		sprintf(key2,"material%d:side",i);
		sprintf(key3,"material%d:bottom",i);	
		r1=dictionary_get(dic, key1, NULL);r2=dictionary_get(dic, key2, NULL);r3=dictionary_get(dic, key3, NULL);
	}
	iniparser_freedict(dic);
}

void bindMaterialSlice(materialSlice_struct* ms)
{
	if(!ms)ms=defaultMaterialSlice;
	applyMTL(ms->img);
}

void getTextureCoordSlice(materialSlice_struct* ms, rectangle_struct* rec, int32* t)
{
	if(!ms)ms=defaultMaterialSlice;
	if(!ms->img)return;
	vect3D p1=vect(0,0,0), p2;
	if(!rec->size.x)
	{
		if(ms->align)p1=vect(inttot16(ms->img->height*rec->position.z-1),inttot16(((ms->img->height*rec->position.y)*HEIGHTUNIT)/(TILESIZE*2)-1),0);
		p2=vect(inttot16(ms->img->height*rec->size.z-1),inttot16(((ms->img->height*rec->size.y)*HEIGHTUNIT)/(TILESIZE*2)-1),0);
		p2=addVect(p1,p2);
		p1=vectDivInt(p1,ms->factor);
		p2=vectDivInt(p2,ms->factor);
		t[3]=TEXTURE_PACK(p1.x, p1.y);
		t[0]=TEXTURE_PACK(p1.x, p2.y);
		t[1]=TEXTURE_PACK(p2.x, p2.y);
		t[2]=TEXTURE_PACK(p2.x, p1.y);
	}else if(!rec->size.y)
	{
		if(ms->align)p1=vect(inttot16(ms->img->width*rec->position.x-1),inttot16(ms->img->height*rec->position.z-1),0);
		p2=vect(inttot16(ms->img->width*rec->size.x-1),inttot16(ms->img->height*rec->size.z-1),0);
		p2=addVect(p1,p2);
		p1=vectDivInt(p1,ms->factor);
		p2=vectDivInt(p2,ms->factor);
		t[0]=TEXTURE_PACK(p1.x, p1.y);
		t[1]=TEXTURE_PACK(p1.x, p2.y);
		t[2]=TEXTURE_PACK(p2.x, p2.y);
		t[3]=TEXTURE_PACK(p2.x, p1.y);
	}else
	{
		if(ms->align)p1=vect(inttot16(ms->img->width*rec->size.x-1),inttot16(((ms->img->height*rec->size.y)*HEIGHTUNIT)/(TILESIZE*2)-1),0);
		p2=vect(inttot16(ms->img->width*rec->size.x-1),inttot16(((ms->img->height*rec->size.y)*HEIGHTUNIT)/(TILESIZE*2)-1),0);
		p2=addVect(p1,p2);
		p1=vectDivInt(p1,ms->factor);
		p2=vectDivInt(p2,ms->factor);
		t[1]=TEXTURE_PACK(p1.x, p1.y);
		t[0]=TEXTURE_PACK(p1.x, p2.y);
		t[3]=TEXTURE_PACK(p2.x, p2.y);
		t[2]=TEXTURE_PACK(p2.x, p1.y);
	}
	
}

char** getMaterialList(int* m, int** cl)
{
	int i, n;
	n=0;
	for(i=0;i<NUMMATERIALS;i++)if(materials[i].used)n++;
	char** l=malloc(sizeof(char*)*(n+1));
	*cl=malloc(sizeof(int)*(n));
	n=0;
	for(i=0;i<NUMMATERIALS;i++)if(materials[i].used){l[n]=malloc(sizeof(char)*(32));(*cl)[n]=i;sprintf(l[n],"material%d",i);n++;}
	l[n]=NULL;
	*m=n;
	return l;
}

material_struct* getMaterial(u16 i){if(i<0||i>NUMMATERIALS)i=0;return &materials[i];}

void freeMaterialList(char** l)
{
	if(!l)return;
	char** l2=l;
	while(*l2){free(*l2);l2++;}
	free(l);
}

void bindMaterial(material_struct* m, rectangle_struct* rec, int32* t)
{
	if(!m)m=defaultMaterial;
	if(!m->used)return;
	materialSlice_struct* ms=m->side;
	if(!rec->size.y)
	{
		if(rec->normal.y>0)ms=m->top;
		else ms=m->bottom;
	}
	unbindMtl();
	bindMaterialSlice(ms);
	getTextureCoordSlice(ms,rec,t);
}