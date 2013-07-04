#include <mex.h>

#include <string>
#include <iostream>
using namespace std;

#include <algonquin/algonquin.h>
#include <algonquin/algonquinnetwork.h>
#include <matlab/convert.h>




const size_t FUNCTION_IDX = 0;
const size_t TYPE_IDX = 1;
const size_t POINTER_IDX = 2;



void processNetwork(const std::string &functionName,
                    int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{

    if (functionName == "create")
    {
        plhs[0] = ptrToMxArray(new AlgonquinNetwork());
        return;
    }

    AlgonquinNetwork *nwk = mxArrayToPtr<AlgonquinNetwork>(prhs[POINTER_IDX]);
    if (functionName == "train")
    {
        mat speech = mxArrayTo<mat>(prhs[POINTER_IDX+1]);
        size_t numSpeechComps = mxArrayTo<int>(prhs[POINTER_IDX+2]);

        mat noise = mxArrayTo<mat>(prhs[POINTER_IDX+3]);
        size_t numNoiseComps = mxArrayTo<int>(prhs[POINTER_IDX+4]);

        size_t maxNumIters = mxArrayTo<int>(prhs[POINTER_IDX+5]);

        nwk->train(speech, numSpeechComps, noise, numNoiseComps, maxNumIters);
        plhs[0] = toMxStruct(nwk->speechDistr());
        plhs[1] = toMxStruct(nwk->noiseDistr());
    }
    else if (functionName == "process")
    {
        vec frame = mxArrayTo<vec>(prhs[POINTER_IDX+1]);
        auto result = nwk->process(frame);
        plhs[0] = toMxArray(*result.first);
        plhs[1] = toMxArray(*result.second);
    }
    else if (functionName == "delete")
        delete nwk;
}




void processGMM(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    double *data = NULL;
    size_t rows;
    size_t cols;
    mxArrayToDoubleArray(prhs[POINTER_IDX], data, rows, cols);

    const size_t numPoints = cols;
    const size_t maxNumIters = 150;
    const size_t numMixtures = 6;

    auto mixture = trainMixture(data, numPoints, numMixtures, maxNumIters);

    double means[numMixtures], precs[numMixtures];
    for (size_t m = 0; m < mixture->means->size(); ++m)
    {
        means[m] = mixture->means->moments(m).mean;
        precs[m] = mixture->precs->moments(m).precision;
    }
    plhs[0] = toMxArray(means, 1, numMixtures);
    plhs[1] = toMxArray(precs, 1, numMixtures);
    plhs[2] = toMxArray(mixture->weights->moments().probs);
}

void processMVGMM(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    mat POINTS = mxArrayTo<mat>(prhs[POINTER_IDX]);
    size_t numMixtures = mxArrayTo<int>(prhs[POINTER_IDX+1]);
    size_t numIters = mxArrayTo<int>(prhs[POINTER_IDX+2]);
    vec assignments = mxArrayTo<vec>(prhs[POINTER_IDX+3]);

    mat means;
    cube sigmas;
    vec weights;
    trainMVMixtureVB(POINTS, numMixtures, numIters, assignments,
                     means, sigmas, weights);
    plhs[0] = toMxArray(means);
    plhs[1] = toMxArray(sigmas);
    plhs[2] = toMxArray(weights);
}



void processAlgonquin(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    mat frames = mxArrayTo<mat>(prhs[POINTER_IDX]);
    mat sMeans = mxArrayTo<mat>(prhs[POINTER_IDX+1]);
    mat sVars = mxArrayTo<mat>(prhs[POINTER_IDX+2]);
    vec sWeights = mxArrayTo<vec>(prhs[POINTER_IDX+3]);

    mat nMeans = mxArrayTo<mat>(prhs[POINTER_IDX+4]);
    mat nVars = mxArrayTo<mat>(prhs[POINTER_IDX+5]);
    vec nWeights = mxArrayTo<vec>(prhs[POINTER_IDX+6]);

    size_t numIters = mxArrayTo<int>(prhs[POINTER_IDX+7]);

    pair<mat,mat> result = vmp::runAlgonquin(sMeans, sVars, sWeights,
                                             nMeans, nVars, nWeights,
                                             frames, numIters);
    plhs[0] = toMxArray(result.first);
    plhs[1] = toMxArray(result.second);
}



void processExample(const string &name, int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    cout << name << endl;

    if (name == "univariate")
    {

    }
    else if (name == "multivariate")
    {

    }
    else if (name == "univariateHierarchical")
    {

    }
}


void processTest(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
}




// the entry point into the function
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs < 1)
    {
        mexErrMsgTxt("Not enough arguments");
        return;
    }

    try
    {
        string functionName(mxArrayToString(prhs[FUNCTION_IDX]));
        string typeName(mxArrayToString(prhs[TYPE_IDX]));

        // here the only possible typeName is Network
        if (typeName == "Network")
            processNetwork(functionName, nlhs, plhs, nrhs, prhs);
        else if (typeName == "GMM")
            processGMM(nlhs, plhs, nrhs, prhs);
        else if (typeName == "MVGMM")
            processMVGMM(nlhs, plhs, nrhs, prhs);
        else if (typeName == "algonquin")
            processAlgonquin(nlhs, plhs, nrhs, prhs);
        else if (typeName == "examples")
            processExample(functionName, nlhs, plhs, nrhs, prhs);
        else if (typeName == "test")
            processTest(nlhs, plhs, nrhs, prhs);
        else
            mexErrMsgTxt("Unsupported operation\n");
    }
    catch (...)
    {
        mexErrMsgTxt("Unknown error occured\n");
    }


}
