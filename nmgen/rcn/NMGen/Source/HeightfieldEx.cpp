/*
 * Copyright (c) 2011 Stephen A. Pratt
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <string>
#include "NMGen.h"
#include "RecastAlloc.h"

struct nmgSpan
{
    unsigned int smin : 13;
    unsigned int smax : 13;
    unsigned int area : 6;
};

extern "C"
{
    EXPORT_API rcHeightfield* nmhfAllocField(const int width
        , const int height
        , const float* bmin
        , const float* bmax
        , const float cs
        , const float ch)
    {
        if (!bmin || !bmax)
            return nullptr;

        rcHeightfield* hf = rcAllocHeightfield();
        if (!hf)
            return nullptr;

        if (rcCreateHeightfield(0
            , *hf
            , width
            , height
            , bmin
            , bmax
            , cs
            , ch))
        {
            return hf;
        }

        rcFreeHeightField(hf);
        return nullptr;
    }

    EXPORT_API void nmhfFreeField(rcHeightfield* hf)
    {
        rcFreeHeightField(hf);
    }

    EXPORT_API bool nmhfRasterizeTriangle(nmgBuildContext* ctx
        , const float* v
        , const unsigned char area
        , rcHeightfield* hf
        , const int flagMergeThr)
    {
        if (!ctx || !v || !hf)
            return false;

        rcRasterizeTriangle(ctx
            , &v[0], &v[3], &v[6]
            , area
            , *hf
            , flagMergeThr);

        return true;
    }

    EXPORT_API bool nmhfRasterizeTriMesh(nmgBuildContext* ctx
        , const float* verts
        , const int nv
        , const int* tris
        , const unsigned char* areas
        , const int nt
        , rcHeightfield* hf
        , const int flagMergeThr)
    {
        if (!ctx || !verts || !tris || !areas || !hf)
            return false;

        rcRasterizeTriangles(ctx
            , verts
            , nv
            , tris
            , areas
            , nt
            , *hf
            , flagMergeThr);

        return true;
    }

    EXPORT_API bool nmhfRasterizeTriMeshShort(nmgBuildContext* ctx
        , const float* verts
        , const int nv
        , const unsigned short* tris
        , const unsigned char* areas
        , const int nt
        , rcHeightfield* hf
        , const int flagMergeThr)
    {
        if (!ctx || !verts || !tris || !areas || !hf)
            return false;

        rcRasterizeTriangles(ctx
            , verts
            , nv
            , tris
            , areas
            , nt
            , *hf
            , flagMergeThr);

        return true;
    }

    EXPORT_API bool nmhfRasterizeTriangles(nmgBuildContext* ctx
        , const float* verts
        , const unsigned char* areas
        , const int nt
        , rcHeightfield* hf
        , const int flagMergeThr)
    {
        if (!ctx || !verts || !areas || !hf)
            return false;

        rcRasterizeTriangles(ctx
            , verts
            , areas
            , nt
            , *hf
            , flagMergeThr);

        return true;
    }

    EXPORT_API bool nmhfFilterLowHangingWalkableObstacles(nmgBuildContext* ctx
        , const int walkableClimb
        , rcHeightfield* hf)
    {
        if (!ctx || !hf)
            return false;

        rcFilterLowHangingWalkableObstacles(ctx, walkableClimb, *hf);

        return true;
    }

    EXPORT_API bool nmhfFilterLedgeSpans(nmgBuildContext* ctx
        , const int walkableHeight
        , const int walkableClimb
        , rcHeightfield* hf)
    {
        if (!ctx || !hf)
            return false;

        rcFilterLedgeSpans(ctx, walkableHeight, walkableClimb, *hf);

        return true;
    }

    EXPORT_API bool nmhfFilterWalkableLowHeightSpans(nmgBuildContext* ctx
        , int walkableHeight
        , rcHeightfield* hf)
    {
        if (!ctx || !hf)
            return false;

        rcFilterWalkableLowHeightSpans(ctx, walkableHeight, *hf);

        return true;
    }

    EXPORT_API int nmhfGetHeightFieldSpanCount(rcHeightfield* hf)
    {
        if (!hf)
            return 0;

        return rcGetHeightFieldSpanCount(0, *hf);
    }

    EXPORT_API int nmhfGetMaxSpansInColumn(rcHeightfield* hf)
    {
        if (!hf)
            return 0;

	    const int w = hf->width;
	    const int h = hf->height;
        int maxCount = 0;
	    for (int y = 0; y < h; ++y)
	    {
		    for (int x = 0; x < w; ++x)
		    {
                int spanCount = 0;
			    for (rcSpan* s = hf->spans[x + y*w]; s; s = s->next)
			    {
				    if (s->area != RC_NULL_AREA)
					    spanCount++;
			    }
                maxCount = rcMax(spanCount, maxCount);
		    }
	    }
	    return maxCount;
    }

    EXPORT_API int nmhfGetSpans(rcHeightfield* hf
        , const int iw
        , const int ih
        , nmgSpan* spans
        , int spanSize)
    {
        if (!hf || !spans
            || iw < 0 || iw > hf->width
            || ih < 0 || ih > hf->height)
        {
            return -1;
        }

        int spanCount = 0;

        const int w = hf->width;
		for (rcSpan* s = hf->spans[iw + ih*w]; s; s = s->next)
		{
			if (s->area != RC_NULL_AREA)
            {
                if (spanCount < spanSize)
                {
                    spans[spanCount].area = s->area;
                    spans[spanCount].smax = s->smax;
                    spans[spanCount].smin = s->smin;
				    spanCount++;
                }
                else
                    // Buffer can't fit anymore.
                    return -1;
            }
		}

        return spanCount;
    }
}