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

package nl.tudelft.granula.modeller.platform.operation;

import nl.tudelft.granula.modeller.Type;
import nl.tudelft.granula.modeller.rule.derivation.SimpleSummaryDerivation;
import nl.tudelft.granula.modeller.rule.linking.UniqueParentLinking;
import nl.tudelft.granula.modeller.rule.visual.MainInfoTableVisualization;

import java.util.ArrayList;

public class LoadGraph extends RealtimeOperationModel {

    public LoadGraph() {
        super(Type.GraphMat, Type.LoadGraph);
    }

    public void loadRules() {
        super.loadRules();

        addLinkingRule(new UniqueParentLinking(Type.GraphMat, Type.Job));

        String summary = "LoadGraph operations loads the graph dataset (in binary format) from disk.";
        addInfoDerivation(new SimpleSummaryDerivation(11, summary));

        addVisualDerivation(new MainInfoTableVisualization(1,
                new ArrayList<String>() {{
                }}));
    }

}