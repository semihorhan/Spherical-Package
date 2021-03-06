#include "layers/uv_resample_layer.h"
#include "cpp/resample.h"

namespace spherical {
namespace cpu {

torch::Tensor EquirectangularToTangentImages(
    torch::Tensor input, torch::Tensor quad_idx, torch::Tensor tex_uv,
    int numTextures, int textureHeight, int textureWidth,
    InterpolationType interpolation) {
  // Useful dimensions to have
  const int64_t batchSize   = input.size(0);
  const int64_t channels    = input.size(1);
  const int64_t inputHeight = input.size(2);
  const int64_t inputWidth  = input.size(3);

  // Initialize output and index mask
  torch::Tensor output = torch::zeros(
      {batchSize, channels, numTextures, textureHeight, textureWidth},
      input.options());

  // Call the CUDA kernel once per batch
  for (int b = 0; b < batchSize; b++) {
    if (input.dtype() == torch::kDouble) {
      ResampleToUVMaps<double>(channels * inputHeight * inputWidth, input[b],
                               quad_idx, tex_uv, channels, numTextures,
                               textureHeight, textureWidth, inputHeight,
                               inputWidth, interpolation, output[b]);
    } else if (input.dtype() == torch::kFloat) {
      ResampleToUVMaps<float>(channels * inputHeight * inputWidth, input[b],
                              quad_idx, tex_uv, channels, numTextures,
                              textureHeight, textureWidth, inputHeight,
                              inputWidth, interpolation, output[b]);
    }
  }

  return output;
}

torch::Tensor EquirectangularFromTangentImages(
    torch::Tensor grad_output, torch::Tensor quad_idx, torch::Tensor tex_uv,
    InterpolationType interpolation) {
  // Useful dimensions to have
  const int64_t batchSize     = grad_output.size(0);
  const int64_t channels      = grad_output.size(1);
  const int64_t numTextures   = grad_output.size(2);
  const int64_t textureHeight = grad_output.size(3);
  const int64_t textureWidth  = grad_output.size(4);
  const int64_t inputHeight   = quad_idx.size(0);
  const int64_t inputWidth    = quad_idx.size(1);

  // Initialize output and index mask
  torch::Tensor input = torch::zeros(
      {batchSize, channels, inputHeight, inputWidth}, grad_output.options());

  // Call the CUDA kernel once per batch
  for (int b = 0; b < batchSize; b++) {
    if (grad_output.dtype() == torch::kDouble) {
      ResampleFromUVMaps<double>(
          channels * inputHeight * inputWidth, grad_output[b], quad_idx,
          tex_uv, channels, numTextures, textureHeight, textureWidth,
          inputHeight, inputWidth, interpolation, input[b]);
    } else if (grad_output.dtype() == torch::kFloat) {
      ResampleFromUVMaps<float>(
          channels * inputHeight * inputWidth, grad_output[b], quad_idx,
          tex_uv, channels, numTextures, textureHeight, textureWidth,
          inputHeight, inputWidth, interpolation, input[b]);
    }
  }

  return input;
}

}  // namespace cpu

}  // namespace spherical