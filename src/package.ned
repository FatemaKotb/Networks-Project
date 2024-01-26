package groupproject;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:

        int WS;
        int WR;
        double TO;
        double PD;
        double TD;
        double ED;
        double DD;
        @display("bgb=732,380");
    submodules:
        coordinator: Coordinator {
            @display("p=354,69;i=misc/node2");
        }
        node: Node {
            @display("p=92,232;i=old/ball");
        }
        node1: Node {
            @display("p=621,231;i=old/ball2");
        }
    connections:
        coordinator.gout0 --> node.gin_c;
        coordinator.gout1 --> node1.gin_c;
        node.gout --> node1.gin_r;
        node1.gout --> node.gin_r;
}

