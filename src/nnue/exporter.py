import torch
import struct

INPUT = 768
HIDDEN = 128

class NNUE(torch.nn.Module):
  def __init__(self):
    super().__init__()
    self.fc1 = torch.nn.Linear(INPUT, HIDDEN)
    self.fc2 = torch.nn.Linear(HIDDEN, 1)

model = NNUE()
model.load_state_dict(torch.load("nnue.pt"))
model.eval()

with open("model.nnue", "wb") as f:

  W1 = model.fc1.weight.detach().numpy().T
  B1 = model.fc1.bias.detach().numpy()

  W2 = model.fc2.weight.detach().numpy()[0]
  B2 = model.fc2.bias.detach().numpy()[0]

  for i in range(INPUT):
    for j in range(HIDDEN):
      f.write(struct.pack("f", W1[i][j]))

  for x in B1:
    f.write(struct.pack("f", x))

  for x in W2:
    f.write(struct.pack("f", x))

  f.write(struct.pack("f", float(B2)))