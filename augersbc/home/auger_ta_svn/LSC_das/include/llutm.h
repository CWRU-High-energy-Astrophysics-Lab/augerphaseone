#if !defined(_LLUTM_H_)
#define _LLUTM_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2010-09-20 09:49:56 +0200 (Mon, 20 Sep 2010) $
  $Revision: 4 $

********************************************/


void LLtoUTM( int ReferenceEllipsoid, double Lat, double Longi, 
	      double *UTMNorthing, double *UTMEasting, char* UTMZone) ;
void UTMtoLL(int ReferenceEllipsoid, double UTMNorthing, double UTMEasting,
	     char *UTMZone, double *Lat,  double *Long ) ;

#endif
