#ifndef PIN_UTIL_H
#define	PIN_UTIL_H

#define JOIN(x,y) x ## y
#define ODC(port)  JOIN(_ODC, port)
#define LAT(port)  JOIN(_LAT, port)
#define PORT(port)  JOIN(_R, port)
#define TRIS(port) JOIN(_TRIS, port)

#endif	/* PIN_UTIL_H */

