#include "load_save_jpeg.hpp"

#include <jpeglib.h>
#include <jerror.h>

#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>

struct stream_source_mgr : jpeg_source_mgr {
	std::istream &stream;
	std::vector< JOCTET > buffer;

	stream_source_mgr(jpeg_decompress_struct *cinfo, std::istream &stream_) : stream(stream_) {
		next_input_byte = nullptr;
		bytes_in_buffer = 0;
		init_source = init_source_static;
		fill_input_buffer = fill_input_buffer_static;
		skip_input_data = skip_input_data_static;
		resync_to_restart = jpeg_resync_to_restart;
		term_source = term_source_static;

		cinfo->src = this;
	}

	static void init_source_static(j_decompress_ptr cinfo) {
		//Do nothing.
	}

	boolean member_fill_input_buffer(jpeg_decompress_struct *cinfo) {
		auto rdbuf = stream.rdbuf();
		if (rdbuf == nullptr) {
			ERREXIT( cinfo, JERR_FILE_READ );
		}
		rdbuf->sgetc(); //look at current character; will trigger underflow()
		std::streamsize avail = rdbuf->in_avail();
		if (avail <= 0) {
			ERREXIT( cinfo, JERR_FILE_READ );
		}
		buffer.resize(avail);
		if (avail != rdbuf->sgetn(reinterpret_cast< char * >(buffer.data()), buffer.size())) {
			assert(false && "in_avail should always tell the truth.");
		}
		next_input_byte = buffer.data();
		bytes_in_buffer = buffer.size();

		return TRUE;
	}

	static boolean fill_input_buffer_static(j_decompress_ptr cinfo) {
		return reinterpret_cast< stream_source_mgr * >(cinfo->src)->member_fill_input_buffer(cinfo);
	}

	//generic skip handler:
	static void skip_input_data_static(j_decompress_ptr cinfo, long num_bytes) {
		while (num_bytes > 0) {
			if (cinfo->src->bytes_in_buffer == 0) {
				cinfo->src->fill_input_buffer(cinfo);
				assert(cinfo->src->bytes_in_buffer > 0);
			}
			size_t advance = std::min< size_t >(std::max< long >(0,num_bytes), cinfo->src->bytes_in_buffer);
			cinfo->src->next_input_byte += advance;
			cinfo->src->bytes_in_buffer -= advance;
			num_bytes -= advance;
		}
	}

	static void term_source_static(j_decompress_ptr cinfo) {
		//Do nothing.
	}
};

//error handler (looooosely) based on libjpeg example.c:

void throw_error_exit(j_common_ptr cinfo) {
	std::vector< char > buffer(JMSG_LENGTH_MAX);
	cinfo->err->format_message(cinfo, &buffer[0]);
	throw std::runtime_error("libjpeg reports '" + std::string(buffer.data()) + "'");
}
bool load_jpeg(std::string filename, unsigned int *width_, unsigned int *height_, std::vector< uint32_t > *data_, OriginLocation origin) {
	std::ifstream in_stream(filename, std::ios::binary);
	return load_jpeg(in_stream, width_, height_, data_, origin);
}

bool load_jpeg(std::istream &in_stream, unsigned int *width_, unsigned int *height_, std::vector< uint32_t > *data_, OriginLocation origin) {
	assert(width_);
	auto &width = *width_;
	assert(height_);
	auto &height = *height_;
	assert(data_);
	auto &data = *data_;

	width = 0;
	height = 0;
	data.clear();
	
	//Based on https://github.com/libjpeg-turbo/libjpeg-turbo/blob/master/libjpeg.txt
	
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
		
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = throw_error_exit;
	jpeg_create_decompress(&cinfo);
	
	try {
		//set data source to in_stream:
		stream_source_mgr src(&cinfo, in_stream);
	
		//read header:
		jpeg_read_header(&cinfo, TRUE);
	
		//note -- convert colorspace to RGBA:
		cinfo.out_color_space = JCS_EXT_RGBA;
	
		//decompression:
	
		jpeg_start_decompress(&cinfo);
	
		width = cinfo.output_width;
		height = cinfo.output_height;
		data.resize(width * height);
	
		std::vector< JSAMPROW > row_ptrs;
		row_ptrs.reserve(height);
	
		static_assert(sizeof(JSAMPLE) == 1, "libjpeg samples should be one byte.");
		if (origin == UpperLeftOrigin) {
			for (uint32_t r = 0; r < height; ++r) {
				row_ptrs.emplace_back(reinterpret_cast< JSAMPLE * >(&data[r*width]));
			}
		} else if (origin == LowerLeftOrigin) {
			for (uint32_t r = 0; r < height; ++r) {
				row_ptrs.emplace_back(reinterpret_cast< JSAMPLE * >(&data[(height-1-r)*width]));
			}
		} else {
			assert(false && "Invalid origin.");
		}
	
		while (cinfo.output_scanline < height) {
			jpeg_read_scanlines(&cinfo, &row_ptrs[cinfo.output_scanline], height - cinfo.output_scanline);
		}
		jpeg_finish_decompress(&cinfo);
		
		//clean up:
		jpeg_destroy_decompress(&cinfo);
	
		return true;
	} catch (std::exception &e) {
		jpeg_destroy_decompress(&cinfo);

		std::cout << "Error loading: " << e.what() << std::endl;
		return false;
	}
}


