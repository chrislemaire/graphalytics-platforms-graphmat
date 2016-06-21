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
import nl.tudelft.graphalytics.graphmat.reporting.logging.GraphMatLogger;
import nl.tudelft.granula.modeller.platform.GraphMat;
import nl.tudelft.granula.modeller.job.JobModel;
import java.nio.file.Path;

/**
 * GraphMat platform integration for the Graphalytics benchmark.
 */
public final class GraphMatGranulaPlatform extends GraphMatPlatform implements GranulaAwarePlatform {


	public GraphMatGranulaPlatform() {
		super();
		BINARY_DIRECTORY = "./bin/granula";
	}

	@Override
	public void preBenchmark(Benchmark benchmark, Path logDirectory) {
		GraphMatLogger.startPlatformLogging(logDirectory.resolve("OperationLog").resolve("driver.logs"));
	}

	@Override
	public void postBenchmark(Benchmark benchmark, Path logDirectory) {
		GraphMatLogger.stopPlatformLogging();
	}

	@Override
	public JobModel getPerformanceModel() {
		return new JobModel(new GraphMat());
	}

}
