#include "mesh_def.h"

#include <vcg/complex/algorithms/geodesic.h>
#include <vcg/complex/algorithms/stat.h>

using namespace vcg;

void ColorizeByVertexQuality(uintptr_t meshptr, float qMin, float qMax, float perc, bool zerosym)
{
	MyMesh &m = *((MyMesh*) meshptr);
	
	bool usePerc = (perc > 0);
	Distribution<float> H;
	tri::Stat<MyMesh>::ComputePerVertexQualityDistribution(m, H);
	float percLo = H.Percentile(perc/100.0f);
	float percHi = H.Percentile(1.0f - (perc/100.0f));
	
	if (qMin == qMax) {
		std::pair<float, float> minmax = tri::Stat<MyMesh>::ComputePerVertexQualityMinMax(m);
		qMin = minmax.first;
		qMax = minmax.second;
	}

	if (zerosym) {
		qMin = std::min(qMin, -math::Abs(qMax));
		qMax = -qMin;
		percLo = std::min(percLo, -math::Abs(percHi));
		percHi = -percLo;
	}

	if (usePerc) {
		tri::UpdateColor<MyMesh>::PerVertexQualityRamp(m, percLo, percHi);
		printf("Quality Range: %f %f; Used (%f %f) percentile (%f %f)\n", H.Min(), H.Max(), percLo, percHi, perc, (100.0f-perc));
	} else {
		tri::UpdateColor<MyMesh>::PerVertexQualityRamp(m, qMin, qMax);
		printf("Quality Range: %f %f; Used (%f %f)\n", H.Min(), H.Max(), qMin, qMax);
	}
}

void ColorizeByBorderDistance(uintptr_t meshptr)
{
	MyMesh &m = *((MyMesh*) meshptr);
	tri::UpdateTopology<MyMesh>::VertexFace(m);
	tri::UpdateFlags<MyMesh>::FaceBorderFromVF(m);
	tri::UpdateFlags<MyMesh>::VertexBorderFromFaceBorder(m);

	bool ret = tri::Geodesic<MyMesh>::DistanceFromBorder(m);
	// Clean up
	int unreachedCnt = 0;
	for (MyMesh::VertexIterator vi = m.vert.begin(); vi != m.vert.end(); ++vi) {
		if (vi->Q() == std::numeric_limits<float>::max()) {
			unreachedCnt++;
			vi->Q() = 0;
		}
	}
	if(unreachedCnt > 0) printf("Warning: %i vertices unreachable from borders\n", unreachedCnt);

	if (ret) {
		tri::UpdateColor<MyMesh>::PerVertexQualityRamp(m);
	} else {
		printf("Mesh has no borders. No geodesic distance was computed.\n");
	}
}

void ColorPluginTEST()
{

}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(ColorizePlugin) {
	emscripten::function("ColorizeByVertexQuality", &ColorizeByVertexQuality);
	emscripten::function("ColorizeByBorderDistance", &ColorizeByBorderDistance);
}
#endif
