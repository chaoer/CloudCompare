//##########################################################################
//#                                                                        #
//#                            CLOUDCOMPARE                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#          COPYRIGHT: EDF R&D / TELECOM ParisTech (ENST-TSI)             #
//#                                                                        #
//##########################################################################

#include "ccIndexedTransformation.h"

//System
#include <math.h>
#include <string.h>
#include <assert.h>

ccIndexedTransformation::ccIndexedTransformation()
	: ccGLMatrix()
	, m_index(0)
{
}

ccIndexedTransformation::ccIndexedTransformation(const ccGLMatrix& matrix)
	: ccGLMatrix(matrix)
	, m_index(0)
{
}

ccIndexedTransformation::ccIndexedTransformation(const ccGLMatrix& matrix, double index)
	: ccGLMatrix(matrix)
	, m_index(index)
{
}

ccIndexedTransformation::ccIndexedTransformation(const ccIndexedTransformation& trans)
	: ccGLMatrix(trans)
	, m_index(trans.m_index)
{
}

bool ccIndexedTransformation::toAsciiFile(const char* filename) const
{
	FILE* fp = fopen(filename,"wt");
	if (!fp)
		return false;

	//save index first
	if (fprintf(fp,"%.12f\n",m_index) < 1)
	{
		fclose(fp);
		return false;
	}
	
	for (unsigned i=0; i<4; ++i)
	{
		if (fprintf(fp,"%f %f %f %f\n",m_mat[i],m_mat[i+4],m_mat[i+8],m_mat[i+12]) < 4)
		{
			fclose(fp);
			return false;
		}
	}

	fclose(fp);

	return true;
}

bool ccIndexedTransformation::fomAsciiFile(const char* filename)
{
	FILE* fp = fopen(filename,"rt");
	if (!fp)
		return false;

	//read index
	if (fscanf(fp,"%lf\n",&m_index) < 1)
	{
		fclose(fp);
		return false;
	}
	
	float* mat = m_mat;
	for (unsigned i=0; i<4; ++i)
	{
		if (fscanf(fp,"%f %f %f %f\n",mat,mat+4,mat+8,mat+12) < 4)
		{
			fclose(fp);
			return false;
		}
		++mat;
	}
	fclose(fp);

	return true;
}

ccIndexedTransformation ccIndexedTransformation::operator * (const ccGLMatrix& M) const
{
	return ccIndexedTransformation( *static_cast<const ccGLMatrix*>(this) * M, m_index);
}

ccIndexedTransformation& ccIndexedTransformation::operator *= (const ccGLMatrix& M)
{
	ccGLMatrix temp = (*this) * M;
	(*this) = temp;

	return (*this);
}

//ccIndexedTransformation ccIndexedTransformation::operator * (const ccIndexedTransformation& trans) const
//{
//	return ccIndexedTransformation( static_cast<ccGLMatrix*>(this) * trans, m_index);
//}
//
//ccIndexedTransformation& ccIndexedTransformation::operator *= (const ccIndexedTransformation& trans)
//{
//	ccGLMatrix temp = (*this) * M;
//	(*this) = temp;
//
//	return (*this);
//}

ccIndexedTransformation& ccIndexedTransformation::operator += (const CCVector3& T)
{
	*static_cast<ccGLMatrix*>(this) += T;

	return (*this);
}

ccIndexedTransformation& ccIndexedTransformation::operator -= (const CCVector3& T)
{
	*static_cast<ccGLMatrix*>(this) -= T;

	return (*this);
}

ccIndexedTransformation ccIndexedTransformation::transposed() const
{
	ccIndexedTransformation t(*this);
	t.transpose();
	
	return t;
}

ccIndexedTransformation ccIndexedTransformation::inverse() const
{
	ccIndexedTransformation t(*this);
	t.invert();

	return t;
}

ccIndexedTransformation ccIndexedTransformation::Interpolate(	double index,
																const ccIndexedTransformation& trans1,
																const ccIndexedTransformation& trans2)
{
	double dt = trans2.getIndex() - trans1.getIndex();
	if (dt == 0)
	{
		assert(index == trans1.getIndex());
		return trans1;
	}

	//we compute the transformation matrix between trans1 and trans2
	double t = (index - trans1.getIndex())/dt;
	assert(t >= 0 && t <= 1);
	
	ccGLMatrix mat = ccGLMatrix::Interpolate(static_cast<PointCoordinateType>(t),trans1,trans2);

	return ccIndexedTransformation(mat, index);
}

bool ccIndexedTransformation::toFile(QFile& out) const
{
	if (!ccGLMatrix::toFile(out))
		return false;
	
	assert(out.isOpen() && (out.openMode() & QIODevice::WriteOnly));

	//index (dataVersion>=34)
	if (out.write((const char*)&m_index,sizeof(double))<0)
		return WriteError();

	return true;
}

bool ccIndexedTransformation::fromFile(QFile& in, short dataVersion, int flags)
{
	if (!ccGLMatrix::fromFile(in, dataVersion, flags))
		return false;

	assert(in.isOpen() && (in.openMode() & QIODevice::ReadOnly));

	if (dataVersion<34)
		return CorruptError();

	//index (dataVersion>=34)
	if (in.read((char*)&m_index,sizeof(double))<0)
		return ReadError();

	return true;
}