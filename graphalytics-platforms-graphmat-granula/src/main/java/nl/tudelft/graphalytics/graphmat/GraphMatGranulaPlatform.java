/*
 * Copyright 2015 Delft University of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package nl.tudelft.graphalytics.graphmat;

import nl.tudelft.graphalytics.domain.Benchmark;
import nl.tudelft.graphalytics.granula.GranulaAwarePlatform;
import nl.tudelft.granula.modeller.platform.GraphMat;
import nl.tudelft.granula.modeller.job.JobModel;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.nio.file.Path;

/**
 * GraphMat platform integration for the Graphalytics benchmark.
 */
public final class GraphMatGranulaPlatform extends GraphMatPlatform implements GranulaAwarePlatform {

	private static PrintStream console;

	public GraphMatGranulaPlatform() {
		super();
		BINARY_DIRECTORY = "./bin/granula";
	}

	@Override
	public void preBenchmark(Benchmark benchmark, Path logDirectory) {
		startPlatformLogging(logDirectory.resolve("OperationLog").resolve("driver.logs"));
	}

	@Override
	public void postBenchmark(Benchmark benchmark, Path logDirectory) {
		stopPlatformLogging();
	}

	@Override
	public JobModel getJobModel() {
		return new JobModel(new GraphMat());
	}

	public static void startPlatformLogging(Path fileName) {
		console = System.out;
		try {
			File file = null;
			file = fileName.toFile();
			file.getParentFile().mkdirs();
			file.createNewFile();
			FileOutputStream fos = new FileOutputStream(file);
			PrintStream ps = new PrintStream(fos);
			System.setOut(ps);
		} catch(Exception e) {
			e.printStackTrace();
			throw new IllegalArgumentException("cannot redirect to output file");
		}
	}

	public static void stopPlatformLogging() {
		System.setOut(console);
	}
}
