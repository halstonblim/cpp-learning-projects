#include "mc/MonteCarloEngine.h"
#include <random>

namespace derivlib::mc {

MonteCarloEngine::MonteCarloEngine(std::unique_ptr<models::IProcessModel> model, std::size_t num_paths, std::size_t num_steps) :
    model_(std::move(model)),
    num_paths_(num_paths),
    num_steps_(num_steps)
{
    std::random_device rd;
    rng_.seed(rd());
}

}