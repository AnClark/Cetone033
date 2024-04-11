#pragma once

void	wave2str		(int wave, char* text);
void	myfloat2string	(float val, char* text);
void	bool2string		(bool val, char* text);
void    int2string      (int val, char* text, int maxLen);

int		c_val2fine		(float value);
float	c_fine2val		(int value);
int		c_val2coarse	(float value);
float	c_coarse2val	(int value);
float	c_val2cutoff	(float value);
float	c_cutoff2val	(float value);
bool	c_val2bool		(float value);
float	c_bool2val		(bool value);

int		pf2i			(float val, int max);
float	pi2f			(int val, int max);

inline int truncate(float flt)
{
#if 0       // Inline ASM is not supported by all compilers & platforms!
    int i;
    asm
    (R"(
        fld (flt);
        fistp (i);
    )");
    return i;
#else
    return static_cast<int>(flt);
#endif
}
