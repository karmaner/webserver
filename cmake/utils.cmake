function(force_redefine_file_macro_for_sources targetname)
	#获取当前目标的所有源文件
	get_target_property(source_files "${targetname}" SOURCES)
	foreach(sourcefile ${source_files})
		# 获取源文件当前的编译定义列表
		get_property(defs SOURCE "${sourcefile}"
			PROPERTY COMPILE_DEFINITIONS)
		# 获取源文件在项目目录中的相对路径
		get_filename_component(filepath "${sourcefile}" ABSOLUTE)
		string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
		list(APPEND defs "__FILE__=\"${relpath}\"")
		# 设置更新后的编译定义列表
		set_property(
			SOURCE "${sourcefile}"
			PROPERTY COMPILE_DEFINITIONS ${defs}
			)
	endforeach()
endfunction()

function(ragelmaker src_rl outputlist outputdir)
	#创建一个自定义构建步骤，调用ragel处理提供的src_rl文件。
	#输出.cpp文件将被追加到传递给outputlist的变量中。

	get_filename_component(src_file ${src_rl} NAME_WE)

	set(rl_out ${outputdir}/${src_file}.rl.cpp)

	#在函数内部添加到列表中需要特别注意，我们不能使用list(APPEND...)
	#因为结果只在局部范围内有效
	set(${outputlist} ${${outputlist}} ${rl_out} PARENT_SCOPE)

	#警告："-S -M -l -C -T0  --error-format=msvc"这些选项是为了匹配现有的Windows调用
	#我们可能需要为mac和linux使用不同的选项
	add_custom_command(
		OUTPUT ${rl_out}
		COMMAND cd ${outputdir}
		COMMAND ragel ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl} -o ${rl_out} -l -C -G2  --error-format=msvc
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl}
		)
	set_source_files_properties(${rl_out} PROPERTIES GENERATED TRUE)
endfunction(ragelmaker)


function(protobufmaker src_proto outputlist outputdir)
	#创建一个自定义构建步骤，调用ragel处理提供的src_rl文件。
	#输出.cpp文件将被追加到传递给outputlist的变量中。

	get_filename_component(src_file ${src_proto} NAME_WE)
	get_filename_component(src_path ${src_proto} PATH)

	set(protobuf_out ${outputdir}/${src_path}/${src_file}.pb.cc)

	#在函数内部添加到列表中需要特别注意，我们不能使用list(APPEND...)
	#因为结果只在局部范围内有效
	set(${outputlist} ${${outputlist}} ${protobuf_out} PARENT_SCOPE)

	add_custom_command(
		OUTPUT ${protobuf_out}
		COMMAND protoc --cpp_out=${outputdir} -I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${src_proto}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src_proto}
		)
	set_source_files_properties(${protobuf_out} PROPERTIES GENERATED TRUE)
endfunction(protobufmaker)


function(webserver_add_executable targetname srcs depends libs)
	add_executable(${targetname} ${srcs})
	add_dependencies(${targetname} ${depends})
	force_redefine_file_macro_for_sources(${targetname})
	target_link_libraries(${targetname} ${libs})
endfunction()
