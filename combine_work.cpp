#include "CImg.h"
#include "color_convert.hpp"
#include <fstream>
#include <iostream>
#include <vector>

bool file_exists(std::string filename) {
  std::cout << "Checking file " << filename << "\n";
  std::fstream f(filename);
  bool ans = f.good();
  f.close();
  return ans;
}

class out {
public:
  bool p;
  long n_iter;
  double abs_z;

  out() {}
  out(bool is_part, long n_itera, double z_abs) {
    p = is_part;
    n_iter = n_itera;
    abs_z = z_abs;
  }
};

std::vector<std::string> tokenize(std::string s) {
  // Tokenize on the character ' ', essentially splitting the string into its
  // individual words
  std::vector<std::string> tokens;
  std::string token;
  for (size_t i = 0; i < s.length(); i++) {
    if (s[i] == ' ') {
      tokens.push_back(token);
      token.clear();
      continue;
    }
    token += s[i];
  }
  tokens.push_back(token);
  return tokens;
}

void add_to_vector(std::vector<std::vector<out>> &data, int x_reso, int y_reso,
                   out o) {
  if (data.empty() || data.back().size() == x_reso) {
    data.push_back({o});
    return;
  }

  data.back().push_back(o);
}

void decompress_file(std::string filename) {
  std::string command = "xz -d " + filename;
  std::cout << "Decompressing file " << filename << "\n";
  system(command.c_str());
}

int main(int argc, char **argv) {
  int x_reso = 3840*2;
  int y_reso = 0.5625 * x_reso;

  int frame_no = std::stoi(argv[1]);
  size_t N_ITER = 60;

  std::vector<std::vector<out>> data;
  int file_no = 0;
  bool compressed_exists, uncompressed_exists;
  while ((compressed_exists =
              file_exists("uploads/mb_" + std::to_string(frame_no) + "_" +
                          std::to_string(file_no) + ".txt.xz")) ||
         (uncompressed_exists =
              file_exists("uploads/mb_" + std::to_string(frame_no) + "_" +
                          std::to_string(file_no) + ".txt"))) {
    if (compressed_exists) {
      decompress_file("uploads/mb_" + std::to_string(frame_no) + "_" +
                      std::to_string(file_no) + ".txt.xz");
    }

    std::cout << "Loading file " << file_no << "\n";

    std::fstream f("uploads/mb_" + std::to_string(frame_no) + "_" +
                   std::to_string(file_no) + ".txt");
    std::string line;
    std::getline(f, line);
    N_ITER = std::stoull(line);
    while (std::getline(f, line)) {
      if (line.empty())
        continue;

      auto tokens = tokenize(line);
      bool b = tokens[0] == "1";
      long n_iter;
      double abs_z;
      if (b) {
        n_iter = N_ITER;
        abs_z = 1;
      } else {
        n_iter = std::stol(tokens[1]);
        abs_z = std::stod(tokens[2]);
      }
      out o(b, n_iter, abs_z);
      add_to_vector(data, x_reso, y_reso, o);
    }

    file_no++;
    f.close();
  }

  if (!data.empty()) {
    std::cout << "Loaded data: " << data[0].size() << "x" << data.size()
              << "\n";

    // for (auto &i : data) {
    //   std::cout << i.size() << "\n";
    // }
  } else {
    std::cout << "Data loading failed! Maybe no files exist?\n";
  }

  cimg_library::CImg<unsigned char> image(data[0].size(), data.size(), 1, 3, 0);

  for (size_t i = 0; i < data.size(); ++i) {
    for (size_t j = 0; j < data[i].size(); ++j) {
      long n_iter = data[i][j].n_iter;
      double abs_z = data[i][j].abs_z;
      bool b = data[i][j].p;
      n_iter = n_iter + 1 - log2(log(abs_z));
      HsvColor c1;
      // set colors
      c1.h = 255 * n_iter / N_ITER;
      c1.s = 255;
      c1.v = 0;
      if (n_iter < N_ITER)
        c1.v = 255;

      RgbColor c2 = HsvToRgb(c1);

      // try swap if this doesn't work
      image(j, i, 0, 0) = c2.r;
      image(j, i, 0, 1) = c2.g;
      image(j, i, 0, 2) = c2.b;
    }
  }

  image.save(("renders/mb_" + std::string(argv[1]) + ".bmp").c_str());
}