// SparseMatrix.h: interface for the CSparseMatrix class.
// already modify the mulTransMatMat into linear time
//////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <assert.h>
#include <math.h>

// User-defined tolerancy
#define TOL 0.00005
#define	ZERO_TOL 1e-12

class CMatrixElement 
{
public:
	int i;
	int j;
	double value;
	CMatrixElement *rowNext;
	CMatrixElement *colNext;

	CMatrixElement(int newi=0, int newj=0, double newValue=0.0)
	{
		i = newi;
		j = newj;
		value = newValue;
		rowNext = colNext = 0;
	}

	virtual ~CMatrixElement() { if (rowNext != 0) delete rowNext; }
};

class CSparseMatrix
{
	//protected:
public:

	int numRows;
	int numCols;
	CMatrixElement* *rowList;
	CMatrixElement* *colList;
	double* diagonal;

	double *dr;
	double *drb;
	double *dp;
	double *dpb;
	double *dz;
	double *dzb;
	double *dAp;
	double *dATpb;
public:

	CSparseMatrix(int nRows, int nCols)
	{
		numRows = numCols = 0;
		rowList = colList = NULL;
		diagonal = NULL;
		dr = NULL;
		setDimensions(nRows,nCols);
	}


	~CSparseMatrix()
	{
		Cleanup();
	}

	void Cleanup()
	{
		// Only delete rows since the matrix element deletes rowNext
		for(int i = 0; i < numRows; i++)
			if(rowList[i] != NULL)
				delete rowList[i];

		if(rowList != NULL)  delete [] rowList;  rowList  = NULL;
		if(colList != NULL)  delete [] colList;  colList  = NULL;
		if(diagonal != NULL) delete [] diagonal; diagonal = NULL;
		if (dr != NULL) {
			delete[] dr;
			delete[] drb;
			delete[] dp;
			delete[] dpb;
			delete[] dz;
			delete[] dzb;
			delete[] dAp;
			delete[] dATpb;
		}
	}

	void
		CSparseMatrix::setValues(int numEl, int i[], int j[], double vals[])
	{
		for(int idx = 0; idx < numEl; idx++)
			set1Value(i[idx],j[idx],vals[idx]);
	}

	void
		CSparseMatrix::set1Value(int i, int j, double val)
	{
		// Insertion in rows
        if ( fabs(val) < ZERO_TOL)
        {
            return ;
        }

		CMatrixElement *theElem = new CMatrixElement(i,j,val);
		theElem->rowNext = rowList[i];
		rowList[i] = theElem;

		// Insertion in columns
		theElem->colNext = colList[j];
		colList[j] = theElem;

		// If on the diagonal, store it for fast access
		if(i==j)
		{
			diagonal[i] = val;
		}
	}

	void
		CSparseMatrix::modify1Value(int i, int j, double val)
	{
		CMatrixElement *theElem = GetElement(i,j);

		if(theElem == NULL)
			set1Value(i,j,val);
		else
			theElem->value = val;
	}

	int
		CSparseMatrix::DeleteElement(int i, int j)
		//not fully tested
	{
		CMatrixElement *theElem;
		CMatrixElement *leftElem, *rightElem;
		CMatrixElement *aboveElem, *underElem;
		leftElem = rightElem = aboveElem = underElem = NULL;

		for(theElem = rowList[i]; theElem != NULL; theElem = theElem->rowNext)
		{
			if(theElem->j == j)
			{
				rightElem = theElem->rowNext;
				break;
			}
			leftElem = theElem;
		}

		if (theElem->j != j)
			return -1;
		for(theElem = colList[j]; theElem != NULL; theElem = theElem->colNext)
		{
			if(theElem->i == i)
			{
				underElem = theElem->colNext;
				break;
			}
			aboveElem = theElem;
		}
		if (theElem->i != i)
			return -1;
		if (leftElem == NULL) // Means A(i,j) is the first entry of a row
		{
			rowList[i] = rightElem;
		}
		else // not nessesary to chek rightElem is NULL
		{
			leftElem->rowNext = rightElem;
		}

		if (aboveElem == NULL) // Means A(i,j) is the first entry of a col
		{
			colList[j] = underElem;
		}
		else // not nessesary to chek rightElem is NULL
		{
			aboveElem->colNext = underElem;
		}
		if (i == j)
		{
			diagonal[i] = 0;
		}
		return 1;
	}
	void
		CSparseMatrix::add1Value(int i, int j, double val)
	{
		CMatrixElement *theElem = GetElement(i,j);

		if(theElem == NULL)
		{
			if ( fabs(val) > ZERO_TOL)
			{
				set1Value(i,j,val);
			}
		}
		else
		{
			theElem->value += val;
			if ( fabs(theElem->value) < ZERO_TOL)
			{
				DeleteElement(i,j);
			}
		}

	}

	void
		CSparseMatrix::addOneValue(int i, int j, double val)
	{
		CMatrixElement *theElem = GetElement(i,j);

		if(theElem == NULL)
			set1Value(i,j,val);
		else
			theElem->value += val;
	}

	void
		CSparseMatrix::setRow(int i, CMatrixElement *head)
	{
		// Set it in the row
		rowList[i] = head;
		// And in the column (and diagonal)
		CMatrixElement *theElem;
		for(theElem = rowList[i]; theElem != NULL; theElem = theElem->rowNext)
		{
			theElem->colNext = colList[theElem->j];
			colList[theElem->j] = theElem;
			if(i == theElem->j)
				diagonal[i] = theElem->value;
		}
	}

	void
		CSparseMatrix::setDimensions(int nRows)
	{
		setDimensions(nRows, nRows);
	}

	void
		CSparseMatrix::setDimensions(int nRows, int nCols)
	{
		// Clean up anyway. Safer, since life is a jungle.
		Cleanup();
		numRows = nRows;
		numCols = nCols;
		rowList = new CMatrixElement*[numRows];
		colList = new CMatrixElement*[numCols];
		diagonal = new double[numRows];
		for(int k = 0; k < numRows; k++)
		{
			diagonal[k] = 0.;
			rowList[k] = NULL;
		}
		for(int l = 0; l < numCols; l++)
			colList[l] = NULL;

		dr = new double[numRows];
		drb = new double[numRows];
		dp = new double[numRows];
		dpb = new double[numRows];
		dz = new double[numRows];
		dzb = new double[numRows];
		dAp = new double[numRows];
		dATpb = new double[numRows];
	}

	CMatrixElement*
		CSparseMatrix::GetElement(int i, int j)
	{
		CMatrixElement *theElem;
		for(theElem = rowList[i]; theElem != NULL; theElem = theElem->rowNext)
			if(theElem->j == j)
				return theElem;
		return NULL;
	}

	double
		CSparseMatrix::GetValue(int i, int j)
	{
		CMatrixElement *theElem;
		for(theElem = rowList[i]; theElem != NULL; theElem = theElem->rowNext)
			if(theElem->j == j)
				return theElem->value;
		return 0.0;
	}

	double
		CSparseMatrix::diagonalElement(int i)
	{
		assert(i < numRows);
		return diagonal[i];
	}

	void
		CSparseMatrix::Print()
	{
		CMatrixElement *theElem;
		for(int i = 0; i < numRows; i++)
			for(theElem = rowList[i]; theElem != NULL; theElem = theElem->rowNext)
				printf("i=%d, j=%d: %f\n", theElem->i, theElem->j, theElem->value);
	}

	void
		CSparseMatrix::PrintMathematica(FILE *fp)
	{
		int i, j;
		fprintf(fp,"m = {");
		for(i = 0; i < numRows; i++)
		{
			fprintf(fp,"\n{");
			for(j = 0; j < numCols; j++)
			{
				fprintf(fp,"%f",GetValue(i,j));
				if(j != numCols-1) fprintf(fp,", ");
			}
			fprintf(fp,"}");
			if(i != numRows-1) fprintf(fp,",");
		}
		fprintf(fp,"}\n\n");
	}
	void 
		CSparseMatrix::PrintMathematica_wyz(FILE *fp)
	{
		int i, j;
		fprintf(fp,"This is a %d by %d matrix.\n",numRows,numCols);
		fprintf(fp,"m = {");
		for(i = 0; i < numRows; i++)
		{
			fprintf(fp,"\n{");
			for(j = 0; j < numCols; j++)
			{
				double a = GetValue(i,j);
				if( GetElement(i,j) != NULL)
				{
					fprintf(fp,"%d,%d %e",i,j,GetValue(i,j));
					if(j != numCols-1) fprintf(fp,", ");
				}
			}
			fprintf(fp,"}");
			if(i != numRows-1) fprintf(fp,",");
		}
		fprintf(fp,"}\n\n");
	}

	void 
		CSparseMatrix::PrintMathematica_wyz2(FILE *fp)
	{
		int i;
		fprintf(fp,"m = {");
        for(i = 0; i < numRows; i++)
		{
			fprintf(fp,"\n{");
			for(CMatrixElement * theElem_d = rowList[i]; theElem_d != NULL; theElem_d = theElem_d->rowNext)  //D
			{
				int r_id = theElem_d->i;
				int c_id = theElem_d->j;
				fprintf(fp,"%d,%d %f",r_id,c_id,GetValue(r_id,c_id));
				if(i != numCols-1) fprintf(fp,", ");
			}
			
			fprintf(fp,"}");
			if(i != numRows-1) fprintf(fp,",");
		}
		fprintf(fp,"}\n\n");
	}
	void
		PrintVectorMathematica(FILE *fp, double theVec[], int n)
	{
		int i;
		fprintf(fp,"v = {");
		for(i = 0; i < n; i++)
		{
			fprintf(fp,"%f",theVec[i]);
			if(i != n-1) fprintf(fp,", ");
		}
		fprintf(fp,"}\n\n");
	}


	void
		CSparseMatrix::multMatVec(double *src,
		double *dest)
	{
		assert(src && dest);
		CMatrixElement *theElem = NULL;
		for(int i = 0; i < numRows; i++)
		{
			double sum = 0;
			for(theElem = rowList[i];
				theElem != NULL;
				theElem = theElem->rowNext)
				sum += theElem->value * src[theElem->j];
			dest[i] = sum;
		}
	}
	void 	CSparseMatrix::multMatVec_yz(double *src,
		double * &det)
	{
		double *dest;
		dest = new double [numRows];
		CMatrixElement *theElem = NULL;
		for(int i = 0; i < numRows; i++)
		{
			double sum = 0;
			for(theElem = rowList[i];
				theElem != NULL;
				theElem = theElem->rowNext)
				sum += theElem->value * src[theElem->j];
			dest[i] = sum;
		}
		//for(int i = 0; i < numRows; i++)
		//{
		//	det[i] = dest[i];
		//}
		det = dest;
		//delete [] dest;
	}

    void 	CSparseMatrix::multMatVec_yz(double * src)
    {
        double *dest;
        dest = new double [numRows];
        CMatrixElement *theElem = NULL;
        for(int i = 0; i < numRows; i++)
        {
            double sum = 0;
            for(theElem = rowList[i];
                theElem != NULL;
                theElem = theElem->rowNext)
                sum += theElem->value * src[theElem->j];
            dest[i] = sum;
        }
        for(int i = 0; i < numRows; i++)
        {
        	src[i] = dest[i];
        }
        delete [] dest;
    }

	void
		CSparseMatrix::multTransMatVec(double *src,
		double *dest)
	{
		assert(src && dest);
		double sum;

		CMatrixElement *theElem = NULL;
		for(int j = 0; j < numCols; j++)
		{
			sum = 0.0;
			for(theElem = colList[j]; theElem != NULL; theElem = theElem->colNext)
				sum += theElem->value * src[theElem->i];
			dest[j] = sum;
		}
	}

	void
		CSparseMatrix::multTransMatVec_yz(double *src,
		double *det)
	{
		
		double sum;
		double * dest;
		dest = new double[numCols];
		CMatrixElement *theElem = NULL;
		for(int j = 0; j < numCols; j++)
		{
			sum = 0.0;
			for(theElem = colList[j]; theElem != NULL; theElem = theElem->colNext)
				sum += theElem->value * src[theElem->i];
			dest[j] = sum;
			det[j] = dest[j];
		}
		delete [] dest;
	}

	//void
	//CSparseMatrix::Transpose()
	//{
	//
	//	CSparseMatrix* tempMat = new CSparseMatrix(numCols,numRows);
	//
	//}

    //matrix multiplication: result = this * mat
    CSparseMatrix *
        MultMatrix_bb(CSparseMatrix *mat){
            //check if the size of matrices corresponds
            if(this->numCols != mat->numRows)
                return NULL;

            CSparseMatrix *result =new CSparseMatrix(numRows,mat->numCols);
            CMatrixElement *theElem,*matElem;
            for(int i=0; i<this->numRows; i++){
                for(theElem = this->rowList[i]; theElem != NULL; theElem = theElem->rowNext){
                    int k = theElem->j;
                    for(matElem = mat->rowList[k]; matElem != NULL; matElem = matElem->rowNext){
                        int j=matElem->j;
                        result->add1Value(i,j,theElem->value * matElem->value);

                    }
                }

            }

            for(int i=0; i<result->numRows; i++)
                result->diagonal[i] = result->GetValue(i,i);

            return result;
    }

	void
		CSparseMatrix::multTransMatMat_yz()
	{
		// M = transpose(M)*M  <-> A * B
		CSparseMatrix* tempMat = new CSparseMatrix(numCols, numCols);

		int j;
		CMatrixElement *theElem;
		
		#ifdef ___DEBUG_YZ
			FILE * fp;
			fp = fopen("mat_mup_log.txt","w");
			fclose(fp);
		#endif
		
		for(j = 0; j < numCols; j++)
		{
			theElem = colList[j];
			if (theElem == NULL)
				continue;

			for(CMatrixElement * theElem_e = colList[j]; theElem_e != NULL; theElem_e = theElem_e->colNext)  //D^T: col means A.row
			{
				int r_id = theElem_e->i;
				int tem_rid = theElem_e->j;
				for(CMatrixElement *theElem_r = rowList[r_id]; theElem_r != NULL; theElem_r = theElem_r->rowNext)
				{
					#ifdef ___DEBUG_YZ
						fp = fopen("mat_mup_log.txt","a+");
					#endif

					int tem_cid = theElem_r->j;

					#ifdef ___DEBUG_YZ
						fprintf(fp,"j=%d r_id=%d tem_rid=%d tem_cid=%d\n",j,r_id,tem_rid,tem_cid);
					#endif

					double value = theElem_e->value * theElem_r->value;

					#ifdef ___DEBUG_YZ
						fprintf(fp,"A[%d,%d] = %f, B[%d,%d] = %f, C[%d,%d] += %f\n",tem_rid,r_id,theElem_e->value,r_id,tem_cid,theElem_r->value,tem_rid,tem_cid,value);
					#endif

					tempMat->add1Value(tem_rid,tem_cid,value);

					#ifdef ___DEBUG_YZ
						fclose(fp);
					#endif

				}
			}

		}
		// copy tempMat to this
		//Cleanup();
		setDimensions(numCols);
		rowList = tempMat->rowList;
		colList = tempMat->colList;
		diagonal = tempMat->diagonal;
		// delete tempMat; // must fix that ! -> memory leaks here
	}

	void
		CSparseMatrix::writeToFile(FILE *fp)
	{
		CMatrixElement *theElem;
		for(int i = 0; i < numRows; i++)
			for(theElem = rowList[i]; theElem != NULL; theElem = theElem->rowNext)
				fprintf(fp, "%d %d %lf\n", theElem->i, theElem->j, theElem->value);
	}

	void
		CSparseMatrix::readFromFile(FILE *fp)
	{
		int i,j;
		double value;
		while (fscanf_s(fp, "%d %d %lf", &i,&j,&value) != EOF)
		{
			set1Value(i,j,value);
		}

	}



	void
		CSparseMatrix::multTransMatMat()
	{
		// M = transpose(M)*M
		CSparseMatrix* tempMat = new CSparseMatrix(numCols, numCols);
		double *colVec = new double[numRows];
		double *des_colVec = new double[numCols];

		int i, j;
		CMatrixElement *theElem;

		for(j = 0; j < numCols; j++)
		{
			theElem = colList[j];
			if (theElem == NULL)
				continue;
			// initialize the result
			for(i = 0; i < numCols; i++)
				des_colVec[i] = 0.0;

			// grab column j
			for(i = 0; i < numRows; i++)
				colVec[i] = 0.0;
			for(theElem = colList[j]; theElem != NULL; theElem = theElem->colNext)
				colVec[theElem->i] = theElem->value;

			// compute a column of M
			multTransMatVec(colVec, des_colVec);

			// store in tempMat
			for(i = 0; i < numCols; i++)
				if(fabs(des_colVec[i]) > 0.0001)
					tempMat->set1Value(i,j,des_colVec[i]);
		}

		// copy tempMat to this
		// Cleanup();
		setDimensions(numCols);
		rowList = tempMat->rowList;
		colList = tempMat->colList;
		diagonal = tempMat->diagonal;
		delete [] colVec;
		delete [] des_colVec;
		// delete tempMat; // must fix that ! -> memory leaks here
	}

	void
		CSparseMatrix::AddMatrix(CSparseMatrix *mat)
	{
		int i;
		CMatrixElement *theElem, *matElem;
		for(i = 0; i < numRows; i++)
		{
			for(matElem = mat->rowList[i]; matElem != NULL; matElem = matElem->rowNext)
			{
				theElem = GetElement(matElem->i,matElem->j);
				if(theElem == NULL)
				{
					theElem = new CMatrixElement(matElem->i,matElem->j,matElem->value);
					theElem->rowNext = rowList[i];
					rowList[i] = theElem;
					theElem->colNext = colList[theElem->j];
					colList[theElem->j] = theElem;
				}
				else
					theElem->value += matElem->value;
			}
		}
		for(i = 0; i < numRows; i++)
			diagonal[i] += mat->diagonal[i];
	}

	void
		CSparseMatrix::ScaleRow(int i, double s)
	{
		CMatrixElement *theElem;
		for(theElem = rowList[i]; theElem != NULL; theElem = theElem->rowNext)
			theElem->value *= s;
		diagonal[i] *= s;
	}

	//***************************************
	// preconditionedBiConjugateGradient
	//***************************************
	unsigned int 
		solve(double x[],
		double b[],
		double tol,
		const unsigned int iter_max)
	{


		assert(dr && drb && dp && dpb && dz && dzb && dAp && dATpb);
		double mag_r, mag_rOld, mag_pbAp, mag_Residual, Residual0, alpha, beta;

		multMatVec(x,dAp);
		mag_r = mag_Residual = Residual0 = 0.;
		int i = 0;
		for(i = 0; i < numRows; i++)
		{

			dr[i] = drb[i] = b[i] - dAp[i];
			dp[i] = dpb[i] = dz[i] = dzb[i] = dr[i]/diagonalElement(i);	// Simple preconditioning
			mag_r += drb[i] * dz[i];
			mag_Residual += dz[i] * dz[i];
			Residual0 += b[i]*b[i]/(diagonalElement(i)*diagonalElement(i));
		}

		mag_Residual = Residual0*100; // Force the first iteration anyway.
		if(Residual0 == 0)
			Residual0 = 1.;	// To make it work even if ||b|| = 0
		unsigned int nbIter = 0;
		while(mag_Residual > tol && nbIter < iter_max)
		{
			nbIter++;
			multMatVec(dp,dAp);
			multTransMatVec(dpb,dATpb);
			mag_pbAp = 0.0;
			for(i = 0; i < numRows; i++)
				mag_pbAp += dpb[i] * dAp[i];

			if(mag_pbAp == 0)
			{
				//fprintf(stderr,"OOOOOCH!!! (mag_pbAp==0)\n");
				//return 0;
			}

			if(mag_r == 0 && mag_pbAp == 0)
				alpha = 1;
			else
				alpha = mag_r / mag_pbAp;
			mag_rOld = mag_r;
			mag_r = 0.0;
			for(i = 0; i < numRows; i++)
			{
				x[i] += alpha * dp[i];
				dr[i] -= alpha * dAp[i];
				drb[i] -= alpha * dATpb[i];
				dz[i] = dr[i]/diagonalElement(i);
				dzb[i] = drb[i]/diagonalElement(i);
				mag_r += drb[i] * dz[i];
			}

			if(mag_rOld == 0)
			{
				//fprintf(stderr,"OOOOOCH!!! (mag_rOld==0)\n");
				//return 0;
			}

			if(mag_r == 0 && mag_rOld == 0)
				beta = 1.0;
			else
				beta = mag_r / mag_rOld;
			mag_Residual = 0.;
			for(i = 0; i < numRows; i++)
			{
				dp[i] = dz[i] + beta * dp[i];
				dpb[i] = dzb[i] + beta * dpb[i];
				mag_Residual += dz[i] * dz[i];
			}
		}
		return nbIter;
	}
};

