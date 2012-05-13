#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "corres_problem.h"
#include "closest_point.h"


int main(int argc, char *argv[])
{
    dtCorrespondenceProblem problem;

    const char 
        *source_model = argv[1],  /* source reference model */
        *target_model = argv[2],  /* target reference model */
        *markerpoints = argv[3];  /* vertex constraints specified with Corres!
                                   */
    dt_real_type start, step, end;  /* closest point iteration process -
                                       [start:step:end] */
    if (argc == 5)
    {
        printf("reading data...\n");
        CreateCorrespondenceProblem(&problem,
            source_model, target_model, markerpoints, NULL);

        sscanf(argv[4], "[%lf:%lf:%lf]", &start, &step, &end);
        problem.weight_smooth        = 1.0;
        problem.weight_identity      = 0.01;
        problem.weight_closest_start = start;
        problem.weight_closest_step  = step;
        problem.weight_closest_end   = end;

        SolveCorrespondenceProblem(&problem);

        /* save deformed source model (it should looked like the target 
           reference model) and the correspondece list */
        SaveObjFile("out.obj", &(problem.source_model));
        __dt_SaveTriangleCorrsList("out.tricorrs", &(problem.result_tclist));

        /* done */
        DestroyCorrespondenceProblem(&problem);
    }
    else {
        printf(
            "usage: %s source_ref target_ref markerpt [start:step:end]\n",
            argv[0]);
    }

    return 0;
}
